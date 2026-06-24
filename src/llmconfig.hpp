#ifndef LLMMODULE_H
#define LLMMODULE_H

#include <KCModule>
#include <QStringList>
#include <QUrl>
#include <QWidget>

namespace Ui
{
    class LLMConfigWidget;
} // namespace ui

class c_llm_config : public KCModule
{
    Q_OBJECT

public:
    explicit c_llm_config(QObject *parent, const KPluginMetaData &metaData);
    ~c_llm_config() override;

    void load() override;
    void save() override;
    void defaults() override;

private Q_SLOTS:
    void on_provider_changed(int index);
    void on_settings_changed();
    void on_refresh_models_clicked();

private:
    [[nodiscard]] auto can_discover_models() const -> bool;
    [[nodiscard]] auto models_endpoint() const -> QUrl;
    [[nodiscard]] auto fetch_model_ids(const QUrl &url) const -> QStringList;
    void refresh_model_button_state();
    void set_model_list(const QStringList &models);

    Ui::LLMConfigWidget *m_ui;
    QString m_triggerWord;
};

#endif // LLMMODULE_H
