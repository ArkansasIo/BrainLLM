#pragma once

#include <QDialog>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include "config_manager.h"

namespace BrainLLM {

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    SettingsDialog(ConfigManager& config_manager, QWidget* parent = nullptr);
    ~SettingsDialog() = default;
    
private slots:
    void on_apply_clicked();
    void on_ok_clicked();
    void on_cancel_clicked();
    void on_reset_clicked();
    
private:
    void create_ui();
    void load_settings();
    void save_settings();
    
    ConfigManager& config_manager_;
    
    // Brain Settings
    QSpinBox* layers_spinbox_;
    QSpinBox* neurons_spinbox_;
    QDoubleSpinBox* learning_rate_;
    
    // UI Settings
    QCheckBox* dark_mode_;
    QSpinBox* window_width_;
    QSpinBox* window_height_;
    QDoubleSpinBox* font_size_;
    
    // API Settings
    QLineEdit* api_host_;
    QSpinBox* api_port_;
    QCheckBox* enable_cors_;
    
    // Display
    QComboBox* theme_combo_;
    QComboBox* log_level_;
};

} // namespace BrainLLM
