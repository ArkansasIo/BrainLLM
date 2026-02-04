#include "settings_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QTabWidget>

namespace BrainLLM {

SettingsDialog::SettingsDialog(ConfigManager& config_manager, QWidget* parent)
    : QDialog(parent), config_manager_(config_manager) {
    setWindowTitle("Settings");
    setModal(true);
    resize(500, 600);
    create_ui();
    load_settings();
}

void SettingsDialog::create_ui() {
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    
    // Create tab widget
    QTabWidget* tabs = new QTabWidget();
    
    // Brain Tab
    QWidget* brain_widget = new QWidget();
    QVBoxLayout* brain_layout = new QVBoxLayout(brain_widget);
    
    brain_layout->addWidget(new QLabel("Number of Layers:"));
    layers_spinbox_ = new QSpinBox();
    layers_spinbox_->setRange(1, 20);
    brain_layout->addWidget(layers_spinbox_);
    
    brain_layout->addWidget(new QLabel("Neurons per Layer:"));
    neurons_spinbox_ = new QSpinBox();
    neurons_spinbox_->setRange(16, 1024);
    brain_layout->addWidget(neurons_spinbox_);
    
    brain_layout->addWidget(new QLabel("Learning Rate:"));
    learning_rate_ = new QDoubleSpinBox();
    learning_rate_->setRange(0.0001, 1.0);
    learning_rate_->setSingleStep(0.0001);
    brain_layout->addWidget(learning_rate_);
    
    brain_layout->addStretch();
    tabs->addTab(brain_widget, "Brain");
    
    // UI Tab
    QWidget* ui_widget = new QWidget();
    QVBoxLayout* ui_layout = new QVBoxLayout(ui_widget);
    
    dark_mode_ = new QCheckBox("Dark Mode");
    ui_layout->addWidget(dark_mode_);
    
    ui_layout->addWidget(new QLabel("Window Width:"));
    window_width_ = new QSpinBox();
    window_width_->setRange(800, 3840);
    ui_layout->addWidget(window_width_);
    
    ui_layout->addWidget(new QLabel("Window Height:"));
    window_height_ = new QSpinBox();
    window_height_->setRange(600, 2160);
    ui_layout->addWidget(window_height_);
    
    ui_layout->addWidget(new QLabel("Font Size:"));
    font_size_ = new QDoubleSpinBox();
    font_size_->setRange(8.0, 24.0);
    font_size_->setSingleStep(0.5);
    ui_layout->addWidget(font_size_);
    
    ui_layout->addStretch();
    tabs->addTab(ui_widget, "UI");
    
    // API Tab
    QWidget* api_widget = new QWidget();
    QVBoxLayout* api_layout = new QVBoxLayout(api_widget);
    
    api_layout->addWidget(new QLabel("API Host:"));
    api_host_ = new QLineEdit();
    api_layout->addWidget(api_host_);
    
    api_layout->addWidget(new QLabel("API Port:"));
    api_port_ = new QSpinBox();
    api_port_->setRange(1024, 65535);
    api_layout->addWidget(api_port_);
    
    enable_cors_ = new QCheckBox("Enable CORS");
    api_layout->addWidget(enable_cors_);
    
    api_layout->addStretch();
    tabs->addTab(api_widget, "API");
    
    main_layout->addWidget(tabs);
    
    // Buttons
    QHBoxLayout* button_layout = new QHBoxLayout();
    
    QPushButton* apply_btn = new QPushButton("Apply");
    QPushButton* ok_btn = new QPushButton("OK");
    QPushButton* cancel_btn = new QPushButton("Cancel");
    QPushButton* reset_btn = new QPushButton("Reset to Defaults");
    
    button_layout->addWidget(reset_btn);
    button_layout->addStretch();
    button_layout->addWidget(apply_btn);
    button_layout->addWidget(ok_btn);
    button_layout->addWidget(cancel_btn);
    
    main_layout->addLayout(button_layout);
    
    connect(apply_btn, &QPushButton::clicked, this, &SettingsDialog::on_apply_clicked);
    connect(ok_btn, &QPushButton::clicked, this, &SettingsDialog::on_ok_clicked);
    connect(cancel_btn, &QPushButton::clicked, this, &SettingsDialog::on_cancel_clicked);
    connect(reset_btn, &QPushButton::clicked, this, &SettingsDialog::on_reset_clicked);
}

void SettingsDialog::load_settings() {
    auto brain_config = config_manager_.get_brain_config();
    layers_spinbox_->setValue(brain_config.num_layers);
    neurons_spinbox_->setValue(brain_config.neurons_per_layer);
    learning_rate_->setValue(brain_config.learning_rate);
    
    auto ui_config = config_manager_.get_ui_settings();
    dark_mode_->setChecked(ui_config.dark_mode);
    window_width_->setValue(ui_config.window_width);
    window_height_->setValue(ui_config.window_height);
    font_size_->setValue(ui_config.font_size);
    
    auto api_config = config_manager_.get_api_settings();
    api_host_->setText(QString::fromStdString(api_config.host));
    api_port_->setValue(api_config.port);
    enable_cors_->setChecked(api_config.enable_cors);
}

void SettingsDialog::save_settings() {
    BrainConfig brain_config = config_manager_.get_brain_config();
    brain_config.num_layers = layers_spinbox_->value();
    brain_config.neurons_per_layer = neurons_spinbox_->value();
    brain_config.learning_rate = learning_rate_->value();
    config_manager_.set_brain_config(brain_config);
    
    ConfigManager::UISettings ui_settings = config_manager_.get_ui_settings();
    ui_settings.dark_mode = dark_mode_->isChecked();
    ui_settings.window_width = window_width_->value();
    ui_settings.window_height = window_height_->value();
    ui_settings.font_size = font_size_->value();
    config_manager_.set_ui_settings(ui_settings);
    
    ConfigManager::APISettings api_settings = config_manager_.get_api_settings();
    api_settings.host = api_host_->text().toStdString();
    api_settings.port = api_port_->value();
    api_settings.enable_cors = enable_cors_->isChecked();
    config_manager_.set_api_settings(api_settings);
}

void SettingsDialog::on_apply_clicked() {
    save_settings();
}

void SettingsDialog::on_ok_clicked() {
    save_settings();
    accept();
}

void SettingsDialog::on_cancel_clicked() {
    reject();
}

void SettingsDialog::on_reset_clicked() {
    auto default_brain = ConfigManager::default_brain_config();
    layers_spinbox_->setValue(default_brain.num_layers);
    neurons_spinbox_->setValue(default_brain.neurons_per_layer);
    learning_rate_->setValue(default_brain.learning_rate);
}

} // namespace BrainLLM
