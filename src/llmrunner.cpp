#include "llmrunner.hpp"
#include "llmclient.hpp"
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <QClipboard>
#include <QGuiApplication>

K_PLUGIN_CLASS_WITH_JSON(c_llm_runner, "plasma-runner-llm.json")

namespace
{
    constexpr auto DEFAULT_SYSTEM_PROMPT = "Answer in few lines. Preferably 2-3 sentences.";

    [[nodiscard]] auto match_id_for_prompt(const QString &prompt) -> QString
    {
        return QStringLiteral("llm-response:%1").arg(prompt);
    }
} // namespace

c_llm_runner::c_llm_runner(QObject *parent, const KPluginMetaData &metaData)
    : AbstractRunner(parent, metaData)
{
    load_config();
    const auto trigger_len = m_trigger_word.length() + 2;
    setMinLetterCount(trigger_len);

    // Setup debounce timer to avoid multiple concurrent requests
    m_debounce_timer = new QTimer(this);
    m_debounce_timer->setSingleShot(true);
    m_debounce_timer->setInterval(m_debounce_delay);
    connect(m_debounce_timer, &QTimer::timeout, this, [this]()
            {
        if (!m_pending_prompt.isEmpty()) {
            perform_query(m_pending_prompt, m_pending_context);
        } });
}

void c_llm_runner::load_config()
{
    auto config = KSharedConfig::openConfig(QStringLiteral("krunnerllmrc"));
    auto group = config->group(QStringLiteral("General"));

    m_trigger_word = group.readEntry(QStringLiteral("TriggerWord"), QStringLiteral("llm"));

    auto api_key = group.readEntry(QStringLiteral("ApiKey"), QString());
    auto api_base = group.readEntry(QStringLiteral("ApiBase"), QString());
    auto system_prompt = group.readEntry(QStringLiteral("SystemPrompt"), QString::fromLatin1(DEFAULT_SYSTEM_PROMPT));
    auto provider = group.readEntry(QStringLiteral("Provider"), QStringLiteral("OpenAI"));
    auto model = group.readEntry(QStringLiteral("Model"), QStringLiteral("gpt-4"));
    auto max_tokens = group.readEntry(QStringLiteral("MaxTokens"), 150);
    auto timeout = group.readEntry(QStringLiteral("Timeout"), 30000);
    auto debounce_delay = group.readEntry(QStringLiteral("DebounceDelay"), 800);
    auto show_querying_status = group.readEntry(QStringLiteral("ShowQueryingStatus"), true);

    m_configured = !api_key.isEmpty();

    if (provider == QStringLiteral("OpenAI"))
    {
        m_config.provider = llm::e_provider::OpenAI;
    }
    else if (provider == QStringLiteral("OpenAICompatible"))
    {
        m_config.provider = llm::e_provider::OpenAICompatible;
        m_configured = !api_base.trimmed().isEmpty();
    }
    else if (provider == QStringLiteral("Anthropic"))
    {
        m_config.provider = llm::e_provider::Anthropic;
    }
    else if (provider == QStringLiteral("OpenRouter"))
    {
        m_config.provider = llm::e_provider::OpenRouter;
    }
    else if (provider == QStringLiteral("Gemini"))
    {
        m_config.provider = llm::e_provider::Gemini;
    }
    else if (provider == QStringLiteral("Groq"))
    {
        m_config.provider = llm::e_provider::Groq;
    }

    m_config.apiKey = api_key;
    m_config.apiBase = api_base;
    m_config.systemPrompt = system_prompt;
    m_config.model = model;
    m_config.max_tokens = max_tokens;
    m_config.timeout_ms = timeout;
    m_debounce_delay = debounce_delay;
    m_show_querying_status = show_querying_status;

    // Update timer interval if timer already exists
    if (m_debounce_timer)
    {
        m_debounce_timer->setInterval(m_debounce_delay);
    }
}

auto c_llm_runner::create_client() const -> std::unique_ptr<llm ::c_client>
{
    return std::make_unique<llm::c_client>(m_config);
}

void c_llm_runner::reloadConfiguration()
{
    load_config();
    setMinLetterCount(m_trigger_word.length() + 2);
    m_debounce_timer->setInterval(m_debounce_delay);
    m_pending_prompt.clear();
    m_inflight_prompt.clear();
    m_cached_prompt.clear();
    m_cached_response.clear();
    m_cached_error = {};
    m_cached_is_error = false;
}

void c_llm_runner::match(KRunner::RunnerContext &context)
{
    if (!context.isValid())
    {
        return;
    }

    const auto query = context.query();
    const QString trigger_with_space = m_trigger_word + QStringLiteral(" ");

    if (!query.startsWith(trigger_with_space, Qt::CaseInsensitive))
    {
        return;
    }

    if (!m_configured)
    {
        KRunner::QueryMatch match(this);
        match.setId(QStringLiteral("llm-not-configured"));
        match.setCategoryRelevance(KRunner::QueryMatch::CategoryRelevance::Moderate);
        match.setIconName(QStringLiteral("configure"));
        match.setText(i18n("LLM Runner Not Configured"));
        match.setSubtext(i18n("Please configure your provider settings in KRunner settings"));
        match.setRelevance(1.0);
        context.addMatch(match);
        return;
    }

    const auto prompt = query.mid(trigger_with_space.length()).trimmed();

    if (prompt.isEmpty())
    {
        // Stop any pending query
        m_debounce_timer->stop();
        m_pending_prompt.clear();

        KRunner::QueryMatch match(this);
        match.setId(QStringLiteral("llm-help"));
        match.setCategoryRelevance(KRunner::QueryMatch::CategoryRelevance::Moderate);
        match.setIconName(QStringLiteral("help-about"));
        match.setText(i18n("Ask LLM"));
        match.setSubtext(i18n("Type your question after '%1'", m_trigger_word));
        match.setRelevance(1.0);
        context.addMatch(match);
        return;
    }

    if (prompt == m_cached_prompt)
    {
        if (m_cached_is_error)
        {
            handle_error(m_cached_error, context);
        }
        else
        {
            add_response_match(prompt, m_cached_response, context);
        }
        return;
    }

    if (prompt == m_inflight_prompt)
    {
        if (m_show_querying_status)
        {
            add_querying_match(prompt, context);
        }
        return;
    }

    // Cancel any pending request and schedule a new one
    m_debounce_timer->stop();
    m_pending_prompt = prompt;
    m_pending_context = context;
    m_inflight_prompt = prompt;
    m_cached_prompt.clear();
    m_debounce_timer->start();

    if (m_show_querying_status)
    {
        add_querying_match(prompt, context);
    }
}

void c_llm_runner::perform_query(const QString &prompt, KRunner::RunnerContext &context)
{
    if (!context.isValid())
    {
        return;
    }

    // Perform the actual query
    auto client = create_client();
    auto result = client->send_message(prompt);

    if (!result.has_value())
    {
        m_cached_prompt = prompt;
        m_cached_error = result.error();
        m_cached_is_error = true;
        m_inflight_prompt.clear();
        handle_error(m_cached_error, context);
        return;
    }

    m_cached_prompt = prompt;
    m_cached_response = result.value();
    m_cached_is_error = false;
    m_inflight_prompt.clear();
    add_response_match(prompt, m_cached_response, context);
}

void c_llm_runner::add_response_match(const QString &prompt,
                                      const QString &response,
                                      KRunner::RunnerContext &context)
{
    KRunner::QueryMatch match(this);
    match.setId(match_id_for_prompt(prompt));
    match.setCategoryRelevance(KRunner::QueryMatch::CategoryRelevance::Highest);
    match.setIconName(QStringLiteral("dialog-information"));
    match.setText(response);
    match.setRelevance(1.0);
    match.setData(response);
    match.setMultiLine(true);

    // Add action to copy to clipboard
    KRunner::Action copy_action(QStringLiteral("copy"), QStringLiteral("edit-copy"), i18n("Copy to Clipboard"));
    match.setActions({ copy_action });

    context.addMatch(match);
}

void c_llm_runner::add_querying_match(const QString &prompt, KRunner::RunnerContext &context)
{
    KRunner::QueryMatch querying_match(this);
    querying_match.setId(match_id_for_prompt(prompt));
    querying_match.setCategoryRelevance(KRunner::QueryMatch::CategoryRelevance::Moderate);
    querying_match.setIconName(QStringLiteral("view-refresh"));
    querying_match.setText(i18n("Querying LLM..."));
    querying_match.setSubtext(prompt);
    querying_match.setRelevance(0.9);
    querying_match.setEnabled(false);
    context.addMatch(querying_match);
}

void c_llm_runner::run(const KRunner::RunnerContext &context,
                       const KRunner::QueryMatch &match)
{
    Q_UNUSED(context);

    auto response = match.data().toString();
    if (!response.isEmpty())
    {
        auto *clipboard = QGuiApplication::clipboard();
        clipboard->setText(response);
    }
}

void c_llm_runner::handle_error(const llm::s_error &error, KRunner::RunnerContext &context)
{
    QString error_text;
    QString error_subtext;

    switch (error.code)
    {
    case llm::e_error_code::network_error:
        error_text = i18n("Network Error");
        error_subtext = error.message;
        break;
    case llm::e_error_code::invalid_api_key:
        error_text = i18n("API Key Error");
        error_subtext = error.message.isEmpty() ? i18n("Invalid or missing API key") : error.message;
        break;
    case llm::e_error_code::invalid_response:
        error_text = i18n("Invalid Response");
        error_subtext = error.message;
        break;
    case llm::e_error_code::timeout:
        error_text = i18n("Request Timeout");
        error_subtext = i18n("The request took too long");
        break;
    case llm::e_error_code::rate_limited:
        error_text = i18n("Rate Limited");
        error_subtext = i18n("Too many requests, try again later");
        break;
    }

    KRunner::QueryMatch error_match(this);
    error_match.setId(QStringLiteral("llm-error:%1").arg(error_subtext));
    error_match.setIconName(QStringLiteral("dialog-error"));
    error_match.setText(error_text);
    error_match.setSubtext(error_subtext);
    error_match.setData(error.details.isEmpty() ? error.message : error.details);
    error_match.setMultiLine(true);
    error_match.setRelevance(0.8);
    error_match.setCategoryRelevance(KRunner::QueryMatch::CategoryRelevance::High);

    KRunner::Action copy_action(QStringLiteral("copy"), QStringLiteral("edit-copy"), i18n("Copy Error Details"));
    error_match.setActions({ copy_action });

    context.addMatch(error_match);
}

#include "llmrunner.moc"
