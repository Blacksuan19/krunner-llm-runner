#ifndef LLMRUNNER_HPP
#define LLMRUNNER_HPP

#include "llmclient.hpp"
#include <KRunner/AbstractRunner>
#include <KRunner/Action>
#include <KRunner/QueryMatch>
#include <QTimer>
#include <memory>

class c_llm_runner : public KRunner::AbstractRunner
{
    Q_OBJECT

public:
    c_llm_runner(QObject *parent, const KPluginMetaData &metaData);
    ~c_llm_runner() override = default;

    void match(KRunner::RunnerContext &context) override;
    void run(const KRunner::RunnerContext &context,
             const KRunner::QueryMatch &match) override;

private:
    void load_config();
    [[nodiscard]] auto create_client() const -> std::unique_ptr<llm::c_client>;
    void handle_error(const llm ::s_error &error, KRunner::RunnerContext &context);
    void perform_query(const QString &prompt, KRunner::RunnerContext &context);
    void add_response_match(const QString &prompt, const QString &response, KRunner::RunnerContext &context);
    void add_querying_match(const QString &prompt, KRunner::RunnerContext &context);

    QString m_trigger_word;
    llm::s_config m_config;
    bool m_configured{ false };
    int m_debounce_delay{ 800 };
    QTimer *m_debounce_timer{ nullptr };
    QString m_pending_prompt;
    KRunner::RunnerContext m_pending_context;
    QString m_inflight_prompt;
    QString m_cached_prompt;
    QString m_cached_response;
    llm::s_error m_cached_error{};
    bool m_cached_is_error{ false };
};

#endif // LLMRUNNER_HPP
