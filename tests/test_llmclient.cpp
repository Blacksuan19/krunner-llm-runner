#include "../src/llmclient.hpp"
#include <QSignalSpy>
#include <QString>
#include <QTest>
#include <memory>

class c_test_llm_client : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init_test_case();
    void test_config_creation();
    void test_client_creation();
    void test_error_handling();
    void test_provider_endpoints();
    void test_request_building();
    void test_openai_compatible_config();
    void cleanup_test_case();

private:
    auto create_test_config() -> llm::s_config;
};

void c_test_llm_client::init_test_case()
{
    // Setup test environment
}

void c_test_llm_client::test_config_creation()
{
    llm::s_config config;
    config.provider = llm::e_provider::OpenAI;
    config.apiKey = QStringLiteral("test-key");
    config.apiBase = QStringLiteral("https://proxy.example.com/v1");
    config.systemPrompt = QStringLiteral("Answer briefly.");
    config.model = QStringLiteral("gpt-4");
    config.max_tokens = 100;
    config.timeout_ms = 30000;

    QCOMPARE(config.provider, llm::e_provider::OpenAI);
    QCOMPARE(config.apiKey, QStringLiteral("test-key"));
    QCOMPARE(config.apiBase, QStringLiteral("https://proxy.example.com/v1"));
    QCOMPARE(config.systemPrompt, QStringLiteral("Answer briefly."));
    QCOMPARE(config.model, QStringLiteral("gpt-4"));
    QCOMPARE(config.max_tokens, 100);
    QCOMPARE(config.timeout_ms, 30000);
}

void c_test_llm_client::test_client_creation()
{
    auto config = create_test_config();
    auto client = std::make_unique<llm::c_client>(config);

    QVERIFY(client != nullptr);
}

void c_test_llm_client::test_error_handling()
{
    // Test invalid config
    llm::s_config invalid_config;
    invalid_config.apiKey = QString(); // Empty API key
    invalid_config.provider = llm::e_provider::OpenAI;
    invalid_config.model = QStringLiteral("gpt-4");
    invalid_config.max_tokens = 100;
    invalid_config.timeout_ms = 1000; // Short timeout for testing

    auto client = std::make_unique<llm::c_client>(invalid_config);

    // This will fail with network error since API key is invalid
    // We're just testing that the error handling works
    auto result = client->send_message(QStringLiteral("test query"));

    // Should return an error
    QVERIFY(!result.has_value());
}

void c_test_llm_client::test_provider_endpoints()
{
    // Test that different providers would use different endpoints
    // This is implicit in the implementation, but we can verify config

    llm::s_config openai_config;
    openai_config.provider = llm::e_provider::OpenAI;
    openai_config.apiKey = QStringLiteral("test-key");
    openai_config.model = QStringLiteral("gpt-4");

    llm::s_config anthropic_config;
    anthropic_config.provider = llm::e_provider::Anthropic;
    anthropic_config.apiKey = QStringLiteral("test-key");
    anthropic_config.model = QStringLiteral("claude-3-5-sonnet-20241022");

    llm::s_config openrouter_config;
    openrouter_config.provider = llm::e_provider::OpenRouter;
    openrouter_config.apiKey = QStringLiteral("test-key");
    openrouter_config.model = QStringLiteral("anthropic/claude-3.5-sonnet");

    llm::s_config proxy_config;
    proxy_config.provider = llm::e_provider::OpenAICompatible;
    proxy_config.apiBase = QStringLiteral("https://proxy.example.com/v1");
    proxy_config.model = QStringLiteral("custom-model");

    auto openai_client = std::make_unique<llm::c_client>(openai_config);
    auto anthropic_client = std::make_unique<llm::c_client>(anthropic_config);
    auto openrouter_client = std::make_unique<llm::c_client>(openrouter_config);
    auto proxy_client = std::make_unique<llm::c_client>(proxy_config);

    QVERIFY(openai_client != nullptr);
    QVERIFY(anthropic_client != nullptr);
    QVERIFY(openrouter_client != nullptr);
    QVERIFY(proxy_client != nullptr);
}

void c_test_llm_client::test_request_building()
{
    auto config = create_test_config();
    config.max_tokens = 200;

    QCOMPARE(config.max_tokens, 200);

    // Test that config values are properly set
    QVERIFY(!config.apiKey.isEmpty());
    QVERIFY(!config.model.isEmpty());
    QVERIFY(config.timeout_ms > 0);
}

void c_test_llm_client::test_openai_compatible_config()
{
    llm::s_config config;
    config.provider = llm::e_provider::OpenAICompatible;
    config.apiKey = QString();
    config.apiBase = QStringLiteral("http://localhost:8000/v1");
    config.systemPrompt = QStringLiteral("Use concise answers.");
    config.model = QStringLiteral("local-model");
    config.max_tokens = 200;
    config.timeout_ms = 1000;

    auto client = std::make_unique<llm::c_client>(config);

    QVERIFY(client != nullptr);
    QCOMPARE(config.provider, llm::e_provider::OpenAICompatible);
    QCOMPARE(config.apiBase, QStringLiteral("http://localhost:8000/v1"));
    QCOMPARE(config.systemPrompt, QStringLiteral("Use concise answers."));
    QVERIFY(config.apiKey.isEmpty());
}

void c_test_llm_client::cleanup_test_case()
{
    // Cleanup
}

auto c_test_llm_client::create_test_config() -> llm::s_config
{
    llm::s_config config;
    config.provider = llm::e_provider::OpenAI;
    config.apiKey = QStringLiteral("test-api-key");
    config.apiBase = QString();
    config.systemPrompt = QString();
    config.model = QStringLiteral("gpt-4");
    config.max_tokens = 100;
    config.timeout_ms = 30000;
    return config;
}

QTEST_MAIN(c_test_llm_client)
#include "test_llmclient.moc"
