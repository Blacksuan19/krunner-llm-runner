#include "../src/llmrunner.hpp"
#include <KConfigGroup>
#include <KSharedConfig>
#include <QString>
#include <QTest>

class c_test_llm_runner : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init_test_case();
    void test_trigger_word_detection();
    void test_config_loading();
    void test_empty_query();
    void cleanup_test_case();

private:
    void setup_test_config();
};

void c_test_llm_runner::init_test_case()
{
    setup_test_config();
}

void c_test_llm_runner::setup_test_config()
{
    auto config = KSharedConfig::openConfig(QStringLiteral("krunnerllmrc-test"));
    auto group = config->group(QStringLiteral("General"));

    group.writeEntry(QStringLiteral("TriggerWord"), QStringLiteral("llm"));
    group.writeEntry(QStringLiteral("Provider"), QStringLiteral("OpenAI"));
    group.writeEntry(QStringLiteral("ApiKey"), QStringLiteral("test-key"));
    group.writeEntry(QStringLiteral("ApiBase"), QStringLiteral("https://proxy.example.com/v1"));
    group.writeEntry(QStringLiteral("Model"), QStringLiteral("gpt-4"));
    group.writeEntry(QStringLiteral("MaxTokens"), 150);
    group.writeEntry(QStringLiteral("Timeout"), 30000);

    config->sync();
}

void c_test_llm_runner::test_trigger_word_detection()
{
    QString trigger = QStringLiteral("llm");
    QString query1 = QStringLiteral("llm what is the weather?");
    QString query2 = QStringLiteral("what is the weather?");
    QString query3 = QStringLiteral("llm");

    QVERIFY(query1.startsWith(trigger + QStringLiteral(" "), Qt::CaseInsensitive));
    QVERIFY(!query2.startsWith(trigger + QStringLiteral(" "), Qt::CaseInsensitive));
    QVERIFY(!query3.startsWith(trigger + QStringLiteral(" "), Qt::CaseInsensitive));
}

void c_test_llm_runner::test_config_loading()
{
    auto config = KSharedConfig::openConfig(QStringLiteral("krunnerllmrc-test"));
    auto group = config->group(QStringLiteral("General"));

    auto trigger_word = group.readEntry(QStringLiteral("TriggerWord"), QString());
    auto provider = group.readEntry(QStringLiteral("Provider"), QString());
    auto api_key = group.readEntry(QStringLiteral("ApiKey"), QString());
    auto api_base = group.readEntry(QStringLiteral("ApiBase"), QString());
    auto model = group.readEntry(QStringLiteral("Model"), QString());

    QCOMPARE(trigger_word, QStringLiteral("llm"));
    QCOMPARE(provider, QStringLiteral("OpenAI"));
    QCOMPARE(api_key, QStringLiteral("test-key"));
    QCOMPARE(api_base, QStringLiteral("https://proxy.example.com/v1"));
    QCOMPARE(model, QStringLiteral("gpt-4"));
}

void c_test_llm_runner::test_empty_query()
{
    QString query = QStringLiteral("llm ");
    QString trigger = QStringLiteral("llm ");

    auto prompt = query.mid(trigger.length()).trimmed();
    QVERIFY(prompt.isEmpty());
}

void c_test_llm_runner::cleanup_test_case()
{
    // Cleanup test config
    auto config = KSharedConfig::openConfig(QStringLiteral("krunnerllmrc-test"));
    config->group(QStringLiteral("General")).deleteGroup();
    config->sync();
}

QTEST_GUILESS_MAIN(c_test_llm_runner)
#include "test_llmrunner.moc"
