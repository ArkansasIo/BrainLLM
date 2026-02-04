#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QVBoxLayout>
#include <memory>
#include "llm_engine.h"
#include "rest_server.h"

namespace BrainLLM {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
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
    
private:
    void create_menus();
    void create_status_bar();
    void create_dockable_widgets();
    void update_metrics();
    
    std::shared_ptr<LLMEngine> llm_engine_;
    std::shared_ptr<RestServer> api_server_;
    
    // UI Components
    QWidget* central_widget_;
    QLabel* status_label_;
    QProgressBar* progress_bar_;
    QPushButton* process_button_;
    QPushButton* generate_button_;
    QPushButton* start_api_button_;
    QPushButton* settings_button_;
    QPushButton* train_button_;
    QPushButton* reset_button_;
    QPushButton* memory_button_;
};

} // namespace BrainLLM
