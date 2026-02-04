#include "main_window.h"
#include "editor_widget.h"
#include "console_widget.h"
#include "visualization_widget.h"
#include "menu_system.h"
#include "settings_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QDockWidget>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QTimer>

namespace BrainLLM {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      llm_engine_(std::make_shared<LLMEngine>(ConfigManager::default_brain_config())),
      api_server_(std::make_shared<RestServer>(8080)) {
    
    setWindowTitle("BrainLLM - AI Brain Simulator (Inspired by Chappie)");
    setWindowIcon(QIcon(":/resources/brain.png"));
    resize(1600, 1000);
    
    initialize_ui();
    create_menus();
    create_dockable_widgets();
    create_status_bar();
    
    // Update metrics periodically
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::update_metrics);
    timer->start(500);
}

MainWindow::~MainWindow() {
    if (api_server_ && api_server_->is_running()) {
        api_server_->stop();
    }
}

void MainWindow::initialize_ui() {
    central_widget_ = new QWidget(this);
    setCentralWidget(central_widget_);
    
    QVBoxLayout* main_layout = new QVBoxLayout(central_widget_);
    
    // Create splitter for resizable panels
    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    
    // Editor panel
    EditorWidget* editor = new EditorWidget();
    
    // Console panel
    ConsoleWidget* console = new ConsoleWidget();
    
    splitter->addWidget(editor);
    splitter->addWidget(console);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);
    
    main_layout->addWidget(splitter);
    
    // Control buttons
    QHBoxLayout* button_layout = new QHBoxLayout();
    
    process_button_ = new QPushButton("Process Input");
    generate_button_ = new QPushButton("Generate Response");
    start_api_button_ = new QPushButton("Start API Server");
    train_button_ = new QPushButton("Train");
    reset_button_ = new QPushButton("Reset Brain");
    memory_button_ = new QPushButton("Clear Memory");
    settings_button_ = new QPushButton("Settings");
    
    button_layout->addWidget(process_button_);
    button_layout->addWidget(generate_button_);
    button_layout->addWidget(train_button_);
    button_layout->addWidget(reset_button_);
    button_layout->addWidget(memory_button_);
    button_layout->addWidget(start_api_button_);
    button_layout->addWidget(settings_button_);
    
    main_layout->addLayout(button_layout);
    
    // Progress bar
    progress_bar_ = new QProgressBar();
    progress_bar_->setValue(50);
    main_layout->addWidget(progress_bar_);
    
    // Connect signals
    connect(process_button_, &QPushButton::clicked, this, &MainWindow::on_process_button_clicked);
    connect(generate_button_, &QPushButton::clicked, this, &MainWindow::on_generate_button_clicked);
    connect(start_api_button_, &QPushButton::clicked, this, &MainWindow::on_start_api_clicked);
    connect(settings_button_, &QPushButton::clicked, this, &MainWindow::on_settings_clicked);
    connect(train_button_, &QPushButton::clicked, this, &MainWindow::on_train_clicked);
    connect(reset_button_, &QPushButton::clicked, this, &MainWindow::on_reset_clicked);
    connect(memory_button_, &QPushButton::clicked, this, &MainWindow::on_memory_clear_clicked);
}

void MainWindow::create_menus() {
    MenuSystem menu_system;
    QMenuBar* menu_bar = menuBar();
    
    // File Menu
    QMenu* file_menu = menu_bar->addMenu("&File");
    file_menu->addAction("&New", this, [this]() { /* New action */ });
    file_menu->addAction("&Open", this, [this]() { /* Open action */ });
    file_menu->addAction("&Save", this, [this]() { /* Save action */ });
    file_menu->addSeparator();
    file_menu->addAction("&Exit", this, &QWidget::close);
    
    // Edit Menu
    QMenu* edit_menu = menu_bar->addMenu("&Edit");
    edit_menu->addAction("&Undo");
    edit_menu->addAction("&Redo");
    edit_menu->addSeparator();
    edit_menu->addAction("&Cut");
    edit_menu->addAction("&Copy");
    edit_menu->addAction("&Paste");
    
    // View Menu
    QMenu* view_menu = menu_bar->addMenu("&View");
    view_menu->addAction("&Zoom In");
    view_menu->addAction("&Zoom Out");
    view_menu->addAction("&Reset Zoom");
    view_menu->addSeparator();
    view_menu->addAction("Toggle &Full Screen");
    
    // Brain Menu
    QMenu* brain_menu = menu_bar->addMenu("&Brain");
    brain_menu->addAction("&Initialize", this, [this]() { 
        llm_engine_->initialize(); 
    });
    brain_menu->addAction("&Reset", this, [this]() { 
        llm_engine_->reset(); 
    });
    brain_menu->addAction("&Process Input", this, &MainWindow::on_process_button_clicked);
    brain_menu->addAction("&Generate Response", this, &MainWindow::on_generate_button_clicked);
    brain_menu->addSeparator();
    brain_menu->addAction("&Train Model", this, &MainWindow::on_train_clicked);
    brain_menu->addAction("&View Memory", this, [this]() { /* View memory */ });
    
    // Settings Menu
    QMenu* settings_menu = menu_bar->addMenu("&Settings");
    settings_menu->addAction("&Preferences", this, &MainWindow::on_settings_clicked);
    settings_menu->addAction("&API Configuration", this, [this]() { /* API settings */ });
    settings_menu->addAction("&Brain Configuration", this, [this]() { /* Brain settings */ });
    
    // Help Menu
    QMenu* help_menu = menu_bar->addMenu("&Help");
    help_menu->addAction("&About");
    help_menu->addAction("&Documentation");
}

void MainWindow::create_dockable_widgets() {
    // Visualization Dock
    VisualizationWidget* viz = new VisualizationWidget();
    QDockWidget* viz_dock = new QDockWidget("Brain Visualization", this);
    viz_dock->setWidget(viz);
    addDockWidget(Qt::RightDockWidgetArea, viz_dock);
    
    // Memory Dock
    QDockWidget* memory_dock = new QDockWidget("Memory Status", this);
    QPlainTextEdit* memory_text = new QPlainTextEdit();
    memory_text->setReadOnly(true);
    memory_dock->setWidget(memory_text);
    addDockWidget(Qt::RightDockWidgetArea, memory_dock);
}

void MainWindow::create_status_bar() {
    status_label_ = new QLabel("Status: Idle");
    statusBar()->addWidget(status_label_);
    statusBar()->addPermanentWidget(new QLabel("Ready"));
}

void MainWindow::on_process_button_clicked() {
    status_label_->setText("Status: Processing...");
    // Process implementation
}

void MainWindow::on_generate_button_clicked() {
    status_label_->setText("Status: Generating...");
    // Generate implementation
}

void MainWindow::on_start_api_clicked() {
    if (!api_server_->is_running()) {
        api_server_->set_llm_engine(llm_engine_);
        if (api_server_->start()) {
            start_api_button_->setText("Stop API Server");
            status_label_->setText("Status: API Server Running on port 8080");
        }
    } else {
        api_server_->stop();
        start_api_button_->setText("Start API Server");
        status_label_->setText("Status: API Server Stopped");
    }
}

void MainWindow::on_settings_clicked() {
    ConfigManager config_manager;
    SettingsDialog dialog(config_manager, this);
    dialog.exec();
}

void MainWindow::on_train_clicked() {
    status_label_->setText("Status: Training...");
    std::vector<std::string> training_data = {"Hello", "World", "AI", "Learning"};
    llm_engine_->train(training_data);
    status_label_->setText("Status: Training Complete");
}

void MainWindow::on_reset_clicked() {
    llm_engine_->reset();
    status_label_->setText("Status: Brain Reset");
}

void MainWindow::on_memory_clear_clicked() {
    status_label_->setText("Status: Memory Cleared");
}

void MainWindow::update_metrics() {
    auto metrics = llm_engine_->get_metrics();
    progress_bar_->setValue(static_cast<int>(metrics.accuracy * 100));
}

} // namespace BrainLLM
