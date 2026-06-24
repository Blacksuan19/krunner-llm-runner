#include "llmconfig.hpp"
#include "ui_llmconfig.h"
#include <KConfigGroup>
#include <KPluginFactory>
#include <KSharedConfig>
#include <QComboBox>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QTimer>

K_PLUGIN_CLASS_WITH_JSON(c_llm_config, "kcm_krunner_llm.json")

namespace
{
    constexpr auto DEFAULT_SYSTEM_PROMPT = "Answer in few lines. Preferably 2-3 sentences.";

    [[nodiscard]] auto chat_completions_base(QString api_base) -> QString
    {
        api_base = api_base.trimmed();
        while (api_base.endsWith(QLatin1Char('/')))
        {
            api_base.chop(1);
        }

        if (api_base.endsWith(QStringLiteral("/chat/completions")))
        {
            api_base.chop(QStringLiteral("/chat/completions").length());
        }

        return api_base;
    }
}

c_llm_config::c_llm_config(QObject *parent, const KPluginMetaData &metaData)
    : KCModule(parent, metaData), m_ui(new Ui::LLMConfigWidget)
{
    m_ui->setupUi(widget());
    m_ui->modelLayout->setStretch(0, 2);
    m_ui->modelLayout->setStretch(1, 1);

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
            this, [this]()
            {
                refresh_model_button_state();
                on_settings_changed();
            });
    connect(m_ui->modelCombo->lineEdit(), &QLineEdit::textChanged,
            this, &::c_llm_config::on_settings_changed);
    connect(m_ui->refreshModelsButton, &QPushButton::clicked,
            this, &::c_llm_config::on_refresh_models_clicked);
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
    refresh_model_button_state();
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
    m_ui->modelCombo->setCurrentText(model);

    auto systemPrompt = group.readEntry(QStringLiteral("SystemPrompt"), QString::fromLatin1(DEFAULT_SYSTEM_PROMPT));
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
    group.writeEntry(QStringLiteral("Model"), m_ui->modelCombo->currentText());
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
    m_ui->modelCombo->setCurrentText(QStringLiteral("gpt-4"));
    m_ui->systemPromptEdit->setPlainText(QString::fromLatin1(DEFAULT_SYSTEM_PROMPT));
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
        m_ui->modelCombo->setCurrentText(QStringLiteral("gpt-4"));
        m_ui->modelCombo->lineEdit()->setPlaceholderText(QStringLiteral("e.g., gpt-4, gpt-3.5-turbo"));
        m_ui->apiBaseEdit->setPlaceholderText(QStringLiteral("Only needed for OpenAI-compatible proxies"));
    }
    else if (provider == QStringLiteral("OpenAICompatible"))
    {
        m_ui->modelCombo->setCurrentText(QStringLiteral("gpt-4o-mini"));
        m_ui->modelCombo->lineEdit()->setPlaceholderText(QStringLiteral("Model name exposed by your proxy"));
        m_ui->apiBaseEdit->setPlaceholderText(QStringLiteral("e.g., http://localhost:8000/v1"));
    }
    else if (provider == QStringLiteral("Anthropic"))
    {
        m_ui->modelCombo->setCurrentText(QStringLiteral("claude-3-5-sonnet-20241022"));
        m_ui->modelCombo->lineEdit()->setPlaceholderText(QStringLiteral("e.g., claude-3-5-sonnet-20241022"));
        m_ui->apiBaseEdit->setPlaceholderText(QStringLiteral("Only used by OpenAI-compatible proxies"));
    }
    else if (provider == QStringLiteral("OpenRouter"))
    {
        m_ui->modelCombo->setCurrentText(QStringLiteral("anthropic/claude-3.5-sonnet"));
        m_ui->modelCombo->lineEdit()->setPlaceholderText(QStringLiteral("e.g., anthropic/claude-3.5-sonnet"));
        m_ui->apiBaseEdit->setPlaceholderText(QStringLiteral("Only used by OpenAI-compatible proxies"));
    }
    else if (provider == QStringLiteral("Gemini"))
    {
        m_ui->modelCombo->setCurrentText(QStringLiteral("gemini-2.0-flash-exp"));
        m_ui->modelCombo->lineEdit()->setPlaceholderText(QStringLiteral("e.g., gemini-2.0-flash-exp, gemini-1.5-pro"));
        m_ui->apiBaseEdit->setPlaceholderText(QStringLiteral("Only used by OpenAI-compatible proxies"));
    }
    else if (provider == QStringLiteral("Groq"))
    {
        m_ui->modelCombo->setCurrentText(QStringLiteral("llama-3.3-70b-versatile"));
        m_ui->modelCombo->lineEdit()->setPlaceholderText(QStringLiteral("e.g., llama-3.3-70b-versatile, mixtral-8x7b-32768"));
        m_ui->apiBaseEdit->setPlaceholderText(QStringLiteral("Only used by OpenAI-compatible proxies"));
    }

    refresh_model_button_state();
    on_settings_changed();
}

void c_llm_config::on_settings_changed()
{
    setNeedsSave(true);
}

void c_llm_config::on_refresh_models_clicked()
{
    const auto models = fetch_model_ids(models_endpoint());
    if (!models.isEmpty())
    {
        set_model_list(models);
        m_ui->refreshModelsButton->setToolTip(i18n("Loaded %1 models", models.size()));
        return;
    }

    m_ui->refreshModelsButton->setToolTip(i18n("No models found or model discovery failed"));
}

auto c_llm_config::can_discover_models() const -> bool
{
    const auto provider = m_ui->providerCombo->currentData().toString();
    if (provider == QStringLiteral("OpenAI") || provider == QStringLiteral("OpenRouter"))
    {
        return true;
    }

    return provider == QStringLiteral("OpenAICompatible") && !m_ui->apiBaseEdit->text().trimmed().isEmpty();
}

auto c_llm_config::models_endpoint() const -> QUrl
{
    const auto provider = m_ui->providerCombo->currentData().toString();
    if (provider == QStringLiteral("OpenAI"))
    {
        return QUrl(QStringLiteral("https://api.openai.com/v1/models"));
    }

    if (provider == QStringLiteral("OpenRouter"))
    {
        return QUrl(QStringLiteral("https://openrouter.ai/api/v1/models"));
    }

    return QUrl(chat_completions_base(m_ui->apiBaseEdit->text()) + QStringLiteral("/models"));
}

auto c_llm_config::fetch_model_ids(const QUrl &url) const -> QStringList
{
    if (!url.isValid())
    {
        return {};
    }

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    const auto api_key = m_ui->apiKeyEdit->text();
    if (!api_key.isEmpty())
    {
        request.setRawHeader("Authorization", QStringLiteral("Bearer %1").arg(api_key).toUtf8());
    }

    QNetworkAccessManager manager;
    QEventLoop loop;
    QNetworkReply *reply = manager.get(request);

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QTimer::singleShot(m_ui->timeoutSpin->value() * 1000, &loop, [&loop, reply]()
                       {
                           reply->abort();
                           loop.quit();
                       });
    loop.exec();

    if (reply->error() != QNetworkReply::NoError)
    {
        reply->deleteLater();
        return {};
    }

    const auto data = reply->readAll();
    reply->deleteLater();

    const auto doc = QJsonDocument::fromJson(data);
    if (!doc.isObject())
    {
        return {};
    }

    QStringList models;
    const auto data_array = doc.object().value(QStringLiteral("data")).toArray();
    for (const auto &item : data_array)
    {
        const auto id = item.toObject().value(QStringLiteral("id")).toString();
        if (!id.isEmpty())
        {
            models.append(id);
        }
    }

    models.removeDuplicates();
    models.sort(Qt::CaseInsensitive);
    return models;
}

void c_llm_config::refresh_model_button_state()
{
    m_ui->refreshModelsButton->setEnabled(can_discover_models());
    m_ui->refreshModelsButton->setToolTip(i18n("Fetch models from supported providers"));
}

void c_llm_config::set_model_list(const QStringList &models)
{
    const auto current_model = m_ui->modelCombo->currentText();
    m_ui->modelCombo->clear();
    m_ui->modelCombo->addItems(models);

    if (models.contains(current_model))
    {
        m_ui->modelCombo->setCurrentText(current_model);
    }
    else if (!current_model.isEmpty())
    {
        m_ui->modelCombo->insertItem(0, current_model);
        m_ui->modelCombo->setCurrentIndex(0);
    }
}

#include "llmconfig.moc"
