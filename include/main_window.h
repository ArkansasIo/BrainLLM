#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QTimer>
#include <QFrame>
#include <memory>
#include "llm_engine.h"
#include "rest_server.h"
#include "editor_widget.h"
#include "console_widget.h"
#include "chat_widget.h"
#include "visualization_widget.h"
#include "voice_panel.h"

namespace BrainLLM {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void initialize_ui();

private slots:
    void on_process_button_clicked();
    void on_generate_button_clicked();
    void on_start_api_clicked();
    void on_settings_clicked();
    void on_train_clicked();
    void on_reset_clicked();
    void on_memory_clear_clicked();
    void on_voice_input(const QString& transcript);

private:
    void create_menus();
    void create_status_bar();
    void apply_global_stylesheet();
    QPushButton* make_hud_button(const QString& label, const QString& accent = "#00ffcc");
    QWidget*     make_metric_tile(const QString& title, QLabel*& value_out);
    void         update_metrics();

    std::shared_ptr<LLMEngine>  llm_engine_;
    std::shared_ptr<RestServer> api_server_;

    // Panels
    VisualizationWidget* viz_widget_;
    ChatWidget*          chat_widget_;
    ConsoleWidget*       console_widget_;
    EditorWidget*        editor_widget_;
    VoicePanel*          voice_panel_;

    // Metric labels updated by timer
    QLabel* lbl_acc_;
    QLabel* lbl_conf_;
    QLabel* lbl_cpu_;
    QLabel* lbl_tokens_;
    QLabel* lbl_state_;

    QLabel*       status_label_;
    QProgressBar* progress_bar_;

    QPushButton* process_button_;
    QPushButton* generate_button_;
    QPushButton* start_api_button_;
    QPushButton* settings_button_;
    QPushButton* train_button_;
    QPushButton* reset_button_;
    QPushButton* memory_button_;

    QTimer* metrics_timer_;
};

} // namespace BrainLLM
