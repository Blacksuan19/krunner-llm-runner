#include "llmconfig.hpp"
#include "ui_llmconfig.h"
#include <KConfigGroup>
#include <KPluginFactory>
#include <KSharedConfig>
#include <QComboBox>

K_PLUGIN_CLASS_WITH_JSON(c_llm_config, "kcm_krunner_llm.json")

c_llm_config::c_llm_config(QObject *parent, const KPluginMetaData &metaData)
    : KCModule(parent, metaData), m_ui(new Ui::LLMConfigWidget)
{
    m_ui->setupUi(widget());

    // Populate provider combo box
    m_ui->providerCombo->addItem(QStringLiteral("OpenAI"), QStringLiteral("OpenAI"));
    m_ui->providerCombo->addItem(QStringLiteral("OpenAI Compatible"), QStringLiteral("OpenAICompatible"));
    m_ui->providerCombo->addItem(QStringLiteral("Anthropic"), QStringLiteral("Anthropic"));
    m_ui->providerCombo->addItem(QStringLiteral("OpenRouter"), QStringLiteral("OpenRouter"));
    m_ui->providerCombo->addItem(QStringLiteral("Google Gemini"), QStringLiteral("Gemini"));
    m_ui->providerCombo->addItem(QStringLiteral("Groq"), QStringLiteral("Groq"));

    connect(m_ui->providerCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &::c_llm_config::on_provider_changed);
    connect(m_ui->apiKeyEdit, &QLineEdit::textChanged,
            this, &::c_llm_config::on_settings_changed);
    connect(m_ui->apiBaseEdit, &QLineEdit::textChanged,
            this, &::c_llm_config::on_settings_changed);
    connect(m_ui->modelEdit, &QLineEdit::textChanged,
            this, &::c_llm_config::on_settings_changed);
    connect(m_ui->systemPromptEdit, &QPlainTextEdit::textChanged,
            this, &::c_llm_config::on_settings_changed);
    connect(m_ui->triggerWordEdit, &QLineEdit::textChanged,
            this, &::c_llm_config::on_settings_changed);
    connect(m_ui->maxTokensSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &::c_llm_config::on_settings_changed);
    connect(m_ui->timeoutSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &::c_llm_config::on_settings_changed);
    connect(m_ui->debounceDelaySpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &::c_llm_config::on_settings_changed);

    load();
}

c_llm_config::~c_llm_config()
{
    delete m_ui;
}

void c_llm_config::load()
{
    auto config = KSharedConfig::openConfig(QStringLiteral("krunnerllmrc"));
    auto group = config->group(QStringLiteral("General"));

    m_triggerWord = group.readEntry(QStringLiteral("TriggerWord"), QStringLiteral("llm"));
    m_ui->triggerWordEdit->setText(m_triggerWord);

    auto apiKey = group.readEntry(QStringLiteral("ApiKey"), QString());
    m_ui->apiKeyEdit->setText(apiKey);

    auto apiBase = group.readEntry(QStringLiteral("ApiBase"), QString());
    m_ui->apiBaseEdit->setText(apiBase);

    auto provider = group.readEntry(QStringLiteral("Provider"), QStringLiteral("OpenAI"));
    int providerIndex = m_ui->providerCombo->findData(provider);
    if (providerIndex >= 0)
    {
        m_ui->providerCombo->setCurrentIndex(providerIndex);
    }

    auto model = group.readEntry(QStringLiteral("Model"), QStringLiteral("gpt-4"));
    m_ui->modelEdit->setText(model);

    auto systemPrompt = group.readEntry(QStringLiteral("SystemPrompt"), QString());
    m_ui->systemPromptEdit->setPlainText(systemPrompt);

    auto maxTokens = group.readEntry(QStringLiteral("MaxTokens"), 150);
    m_ui->maxTokensSpin->setValue(maxTokens);

    auto timeout = group.readEntry(QStringLiteral("Timeout"), 30000);
    m_ui->timeoutSpin->setValue(timeout / 1000); // Convert to seconds

    auto debounceDelay = group.readEntry(QStringLiteral("DebounceDelay"), 800);
    m_ui->debounceDelaySpin->setValue(debounceDelay);

    setNeedsSave(false);
}

void c_llm_config::save()
{
    auto config = KSharedConfig::openConfig(QStringLiteral("krunnerllmrc"));
    auto group = config->group(QStringLiteral("General"));

    group.writeEntry(QStringLiteral("TriggerWord"), m_ui->triggerWordEdit->text());
    group.writeEntry(QStringLiteral("ApiKey"), m_ui->apiKeyEdit->text());
    group.writeEntry(QStringLiteral("ApiBase"), m_ui->apiBaseEdit->text().trimmed());
    group.writeEntry(QStringLiteral("Provider"), m_ui->providerCombo->currentData().toString());
    group.writeEntry(QStringLiteral("Model"), m_ui->modelEdit->text());
    group.writeEntry(QStringLiteral("SystemPrompt"), m_ui->systemPromptEdit->toPlainText().trimmed());
    group.writeEntry(QStringLiteral("MaxTokens"), m_ui->maxTokensSpin->value());
    group.writeEntry(QStringLiteral("Timeout"), m_ui->timeoutSpin->value() * 1000); // Convert to ms
    group.writeEntry(QStringLiteral("DebounceDelay"), m_ui->debounceDelaySpin->value());

    config->sync();
    setNeedsSave(false);
}

void c_llm_config::defaults()
{
    m_ui->triggerWordEdit->setText(QStringLiteral("llm"));
    m_ui->apiKeyEdit->clear();
    m_ui->apiBaseEdit->clear();
    m_ui->providerCombo->setCurrentIndex(0); // OpenAI
    m_ui->modelEdit->setText(QStringLiteral("gpt-4"));
    m_ui->systemPromptEdit->clear();
    m_ui->maxTokensSpin->setValue(150);
    m_ui->timeoutSpin->setValue(30);
    m_ui->debounceDelaySpin->setValue(800);

    setNeedsSave(true);
}

void c_llm_config::on_provider_changed(int index)
{
    Q_UNUSED(index);

    auto provider = m_ui->providerCombo->currentData().toString();

    if (provider == QStringLiteral("OpenAI"))
    {
        m_ui->modelEdit->setText(QStringLiteral("gpt-4"));
        m_ui->modelEdit->setPlaceholderText(QStringLiteral("e.g., gpt-4, gpt-3.5-turbo"));
        m_ui->apiBaseEdit->setPlaceholderText(QStringLiteral("Only needed for OpenAI-compatible proxies"));
    }
    else if (provider == QStringLiteral("OpenAICompatible"))
    {
        m_ui->modelEdit->setText(QStringLiteral("gpt-4o-mini"));
        m_ui->modelEdit->setPlaceholderText(QStringLiteral("Model name exposed by your proxy"));
        m_ui->apiBaseEdit->setPlaceholderText(QStringLiteral("e.g., http://localhost:8000/v1"));
    }
    else if (provider == QStringLiteral("Anthropic"))
    {
        m_ui->modelEdit->setText(QStringLiteral("claude-3-5-sonnet-20241022"));
        m_ui->modelEdit->setPlaceholderText(QStringLiteral("e.g., claude-3-5-sonnet-20241022"));
        m_ui->apiBaseEdit->setPlaceholderText(QStringLiteral("Only used by OpenAI-compatible proxies"));
    }
    else if (provider == QStringLiteral("OpenRouter"))
    {
        m_ui->modelEdit->setText(QStringLiteral("anthropic/claude-3.5-sonnet"));
        m_ui->modelEdit->setPlaceholderText(QStringLiteral("e.g., anthropic/claude-3.5-sonnet"));
        m_ui->apiBaseEdit->setPlaceholderText(QStringLiteral("Only used by OpenAI-compatible proxies"));
    }
    else if (provider == QStringLiteral("Gemini"))
    {
        m_ui->modelEdit->setText(QStringLiteral("gemini-2.0-flash-exp"));
        m_ui->modelEdit->setPlaceholderText(QStringLiteral("e.g., gemini-2.0-flash-exp, gemini-1.5-pro"));
        m_ui->apiBaseEdit->setPlaceholderText(QStringLiteral("Only used by OpenAI-compatible proxies"));
    }
    else if (provider == QStringLiteral("Groq"))
    {
        m_ui->modelEdit->setText(QStringLiteral("llama-3.3-70b-versatile"));
        m_ui->modelEdit->setPlaceholderText(QStringLiteral("e.g., llama-3.3-70b-versatile, mixtral-8x7b-32768"));
        m_ui->apiBaseEdit->setPlaceholderText(QStringLiteral("Only used by OpenAI-compatible proxies"));
    }

    on_settings_changed();
}

void c_llm_config::on_settings_changed()
{
    setNeedsSave(true);
}

#include "llmconfig.moc"
