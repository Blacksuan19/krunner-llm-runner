#ifndef LLMCLIENT_HPP
#define LLMCLIENT_HPP

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>
#include <QTimer>

#include <cstdint>
#include <expected>
#include <memory>

namespace llm
{

    enum class e_provider : std::uint8_t
    {
        OpenAI,
        OpenAICompatible,
        Anthropic,
        OpenRouter,
        Gemini,
        Groq
    };

    enum class e_error_code : std::uint8_t
    {
        network_error,
        invalid_api_key,
        invalid_response,
        timeout,
        rate_limited
    };

    struct s_error
    {
        e_error_code code;
        QString message;
        QString details{};
    };

    struct s_message
    {
        QString role;
        QString content;
    };

    struct s_config
    {
        e_provider provider{ e_provider::OpenAI };
        QString apiKey;
        QString apiBase;
        QString systemPrompt;
        QString model;
        int max_tokens{ 150 };
        int timeout_ms{ 30000 };
    };

    class c_client
    {
    public:
        explicit c_client(s_config config);
        ~c_client() = default;

        [[nodiscard]] auto send_message(const QString &prompt) -> std::expected<QString, s_error>;

    private:
        [[nodiscard]] auto build_request() const -> QNetworkRequest;
        [[nodiscard]] auto build_payload(const QString &prompt) const -> QByteArray;
        [[nodiscard]] auto parse_response(const QByteArray &data) const -> std::expected<QString, s_error>;
        [[nodiscard]] auto get_endpoint() const -> QString;

        s_config m_config;
        std::unique_ptr<QNetworkAccessManager> m_networkManager;
    };

} // namespace llm

#endif // LLMCLIENT_HPP
