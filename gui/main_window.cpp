#include "main_window.h"
#include "settings_dialog.h"
#include "config_manager.h"
#include "voice_panel.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QLabel>
#include <QFrame>
#include <QPainter>
#include <QScreen>
#include <QtMath>

namespace BrainLLM {

// ── global stylesheet ─────────────────────────────────────────────────────────
void MainWindow::apply_global_stylesheet()
{
    qApp->setStyleSheet(R"(
        QMainWindow, QWidget {
            background: #020d12;
            color: #90d8c0;
            font-family: Consolas;
        }
        QMenuBar {
            background: #030f16;
            color: #40ffdc;
            font-family: Consolas;
            font-size: 10px;
            border-bottom: 1px solid #1a5040;
            spacing: 4px;
            padding: 2px 8px;
        }
        QMenuBar::item:selected {
            background: #0d3028;
            color: #00ffcc;
        }
        QMenu {
            background: #030f16;
            color: #80d8c0;
            border: 1px solid #1a5040;
            font-family: Consolas;
            font-size: 10px;
        }
        QMenu::item:selected {
            background: #0d3028;
            color: #00ffcc;
        }
        QSplitter::handle {
            background: #1a5040;
            width: 2px;
            height: 2px;
        }
        QScrollBar:vertical {
            background: #020d12;
            width: 6px;
        }
        QScrollBar::handle:vertical {
            background: #1a5040;
            border-radius: 3px;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0;
        }
        QStatusBar {
            background: #030f16;
            color: #407060;
            font-family: Consolas;
            font-size: 9px;
            border-top: 1px solid #1a5040;
        }
        QProgressBar {
            background: #030f16;
            border: 1px solid #1a5040;
            border-radius: 2px;
            text-align: center;
            color: #40ffdc;
            font-family: Consolas;
            font-size: 9px;
            height: 8px;
        }
        QProgressBar::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #005540, stop:1 #00ffcc);
            border-radius: 2px;
        }
        QLineEdit, QPlainTextEdit, QTextEdit {
            background: #020d12;
            color: #80d8c0;
            border: 1px solid #1a5040;
            font-family: Consolas;
            selection-background-color: #0d3028;
        }
        QToolTip {
            background: #030f16;
            color: #40ffdc;
            border: 1px solid #1a5040;
            font-family: Consolas;
            font-size: 9px;
        }
    )");
}

// ── HUD button factory ────────────────────────────────────────────────────────
QPushButton* MainWindow::make_hud_button(const QString& label, const QString& accent)
{
    QPushButton* btn = new QPushButton(label);
    btn->setFixedHeight(28);
    btn->setStyleSheet(
        QString(
            "QPushButton {"
            "  color: %1; background: transparent;"
            "  font-family: Consolas; font-size: 9px; font-weight: bold;"
            "  letter-spacing: 2px;"
            "  border: 1px solid %1; border-radius: 2px;"
            "  padding: 0 10px;"
            "}"
            "QPushButton:hover {"
            "  background: rgba(0,255,200,0.07);"
            "}"
            "QPushButton:pressed {"
            "  background: rgba(0,255,200,0.15);"
            "}").arg(accent));
    return btn;
}

// ── metric tile factory ───────────────────────────────────────────────────────
QWidget* MainWindow::make_metric_tile(const QString& title, QLabel*& value_out)
{
    QWidget* tile = new QWidget;
    tile->setFixedSize(110, 60);
    tile->setStyleSheet(
        "background: #030f16; border: 1px solid #1a5040; border-radius: 3px;");

    QVBoxLayout* vl = new QVBoxLayout(tile);
    vl->setContentsMargins(8, 6, 8, 6);
    vl->setSpacing(2);

    QLabel* lbl_title = new QLabel(title);
    lbl_title->setStyleSheet(
        "color: #306050; font-family: Consolas; font-size: 8px;"
        "letter-spacing: 2px; background: transparent;");
    vl->addWidget(lbl_title);

    value_out = new QLabel("—");
    value_out->setStyleSheet(
        "color: #40ffdc; font-family: Consolas; font-size: 16px;"
        "font-weight: bold; background: transparent;");
    vl->addWidget(value_out);

    return tile;
}

// ── constructor ───────────────────────────────────────────────────────────────
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      llm_engine_(std::make_shared<LLMEngine>(ConfigManager::default_brain_config())),
      api_server_(std::make_shared<RestServer>(8080)),
      metrics_timer_(new QTimer(this))
{
    apply_global_stylesheet();

    setWindowTitle("BrainLLM — Neural Interface");
    resize(1760, 1020);

    // Center on screen
    if (QScreen* scr = QApplication::primaryScreen()) {
        QRect sg = scr->availableGeometry();
        move((sg.width() - width()) / 2, (sg.height() - height()) / 2);
    }

    initialize_ui();
    create_menus();
    create_status_bar();

    connect(metrics_timer_, &QTimer::timeout, this, &MainWindow::update_metrics);
    metrics_timer_->start(500);
}

MainWindow::~MainWindow()
{
    if (api_server_ && api_server_->is_running())
        api_server_->stop();
}

// ── UI layout ─────────────────────────────────────────────────────────────────
void MainWindow::initialize_ui()
{
    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout* root = new QVBoxLayout(central);
    root->setContentsMargins(8, 8, 8, 4);
    root->setSpacing(6);

    // ── TOP: metric tiles ─────────────────────────────────────────────────────
    QHBoxLayout* top_bar = new QHBoxLayout;
    top_bar->setSpacing(6);

    QLabel* brain_title = new QLabel("◈  BRAINLLM  NEURAL  INTELLIGENCE  SYSTEM");
    brain_title->setStyleSheet(
        "color: #40ffdc; font-family: Consolas; font-size: 11px;"
        "font-weight: bold; letter-spacing: 3px; background: transparent;");
    top_bar->addWidget(brain_title);
    top_bar->addStretch();

    top_bar->addWidget(make_metric_tile("ACCURACY",  lbl_acc_));
    top_bar->addWidget(make_metric_tile("CONFIDENCE",lbl_conf_));
    top_bar->addWidget(make_metric_tile("CPU",        lbl_cpu_));
    top_bar->addWidget(make_metric_tile("TOKENS",     lbl_tokens_));
    top_bar->addWidget(make_metric_tile("STATE",      lbl_state_));

    root->addLayout(top_bar);

    // ── MAIN SPLITTER ─────────────────────────────────────────────────────────
    QSplitter* h_split = new QSplitter(Qt::Horizontal);
    h_split->setHandleWidth(3);

    // Left: editor + console stacked
    QSplitter* left_split = new QSplitter(Qt::Vertical);
    left_split->setHandleWidth(3);

    editor_widget_ = new EditorWidget();
    editor_widget_->setMinimumWidth(260);
    editor_widget_->setStyleSheet(
        "background: #020d12; border: 1px solid #1a5040; color: #80d8c0;"
        "font-family: Consolas; font-size: 11px;");

    console_widget_ = new ConsoleWidget();
    console_widget_->setMinimumHeight(140);

    left_split->addWidget(editor_widget_);
    left_split->addWidget(console_widget_);
    left_split->setStretchFactor(0, 3);
    left_split->setStretchFactor(1, 1);

    // Centre: neural sphere visualization
    viz_widget_ = new VisualizationWidget();
    viz_widget_->setMinimumWidth(400);
    viz_widget_->setMinimumHeight(420);
    viz_widget_->setStyleSheet("border: 1px solid #1a5040; border-radius: 4px;");

    // Right: voice panel stacked above chat
    QSplitter* right_split = new QSplitter(Qt::Vertical);
    right_split->setHandleWidth(3);

    voice_panel_ = new VoicePanel();
    voice_panel_->set_llm_engine(llm_engine_);
    voice_panel_->setMinimumHeight(260);
    voice_panel_->setMaximumHeight(380);

    chat_widget_ = new ChatWidget();
    chat_widget_->set_llm_engine(llm_engine_);
    chat_widget_->setMinimumWidth(280);

    right_split->addWidget(voice_panel_);
    right_split->addWidget(chat_widget_);
    right_split->setStretchFactor(0, 0);
    right_split->setStretchFactor(1, 1);

    h_split->addWidget(left_split);
    h_split->addWidget(viz_widget_);
    h_split->addWidget(right_split);
    h_split->setStretchFactor(0, 2);
    h_split->setStretchFactor(1, 3);
    h_split->setStretchFactor(2, 2);

    root->addWidget(h_split, 1);

    // ── BOTTOM: command bar ────────────────────────────────────────────────────
    QWidget* cmd_bar = new QWidget;
    cmd_bar->setFixedHeight(44);
    cmd_bar->setStyleSheet(
        "background: #030f16; border: 1px solid #1a5040; border-radius: 3px;");
    QHBoxLayout* cmd_layout = new QHBoxLayout(cmd_bar);
    cmd_layout->setContentsMargins(10, 0, 10, 0);
    cmd_layout->setSpacing(8);

    process_button_   = make_hud_button("▶ PROCESS");
    generate_button_  = make_hud_button("◆ GENERATE");
    train_button_     = make_hud_button("▲ TRAIN",    "#00ccff");
    reset_button_     = make_hud_button("↺ RESET",    "#ff8040");
    memory_button_    = make_hud_button("⊘ CLR MEM",  "#cc6060");
    start_api_button_ = make_hud_button("⬡ API SRV",  "#60a0ff");
    settings_button_  = make_hud_button("⚙ SETTINGS", "#80a090");

    cmd_layout->addWidget(process_button_);
    cmd_layout->addWidget(generate_button_);
    cmd_layout->addWidget(train_button_);

    // Divider
    QFrame* sep1 = new QFrame;
    sep1->setFrameShape(QFrame::VLine);
    sep1->setStyleSheet("color: #1a5040;");
    cmd_layout->addWidget(sep1);

    cmd_layout->addWidget(reset_button_);
    cmd_layout->addWidget(memory_button_);

    QFrame* sep2 = new QFrame;
    sep2->setFrameShape(QFrame::VLine);
    sep2->setStyleSheet("color: #1a5040;");
    cmd_layout->addWidget(sep2);

    cmd_layout->addWidget(start_api_button_);
    cmd_layout->addStretch();
    cmd_layout->addWidget(settings_button_);

    root->addWidget(cmd_bar);

    // ── Progress bar ──────────────────────────────────────────────────────────
    progress_bar_ = new QProgressBar;
    progress_bar_->setFixedHeight(6);
    progress_bar_->setTextVisible(false);
    progress_bar_->setValue(50);
    root->addWidget(progress_bar_);

    // ── Signal connections ────────────────────────────────────────────────────
    connect(process_button_,  &QPushButton::clicked, this, &MainWindow::on_process_button_clicked);
    connect(generate_button_, &QPushButton::clicked, this, &MainWindow::on_generate_button_clicked);
    connect(start_api_button_,&QPushButton::clicked, this, &MainWindow::on_start_api_clicked);
    connect(settings_button_, &QPushButton::clicked, this, &MainWindow::on_settings_clicked);
    connect(train_button_,    &QPushButton::clicked, this, &MainWindow::on_train_clicked);
    connect(reset_button_,    &QPushButton::clicked, this, &MainWindow::on_reset_clicked);
    connect(memory_button_,   &QPushButton::clicked, this, &MainWindow::on_memory_clear_clicked);

    // Voice panel: mic transcript feeds the chat input
    connect(voice_panel_, &VoicePanel::voice_input_ready,
            this, &MainWindow::on_voice_input);
}

// ── menus ─────────────────────────────────────────────────────────────────────
void MainWindow::create_menus()
{
    QMenuBar* mb = menuBar();

    QMenu* file = mb->addMenu("FILE");
    file->addAction("New",  this, [](){ });
    file->addAction("Open", this, [](){ });
    file->addAction("Save", this, [](){ });
    file->addSeparator();
    file->addAction("Exit", this, &QWidget::close);

    QMenu* brain = mb->addMenu("BRAIN");
    brain->addAction("Initialize", this, [this](){ llm_engine_->initialize(); });
    brain->addAction("Reset",      this, &MainWindow::on_reset_clicked);
    brain->addAction("Process",    this, &MainWindow::on_process_button_clicked);
    brain->addAction("Generate",   this, &MainWindow::on_generate_button_clicked);
    brain->addSeparator();
    brain->addAction("Train",      this, &MainWindow::on_train_clicked);
    brain->addAction("Clear Memory", this, &MainWindow::on_memory_clear_clicked);

    QMenu* api = mb->addMenu("API");
    api->addAction("Toggle Server", this, &MainWindow::on_start_api_clicked);

    QMenu* settings = mb->addMenu("SETTINGS");
    settings->addAction("Preferences", this, &MainWindow::on_settings_clicked);

    QMenu* help = mb->addMenu("HELP");
    help->addAction("About BrainLLM");
    help->addAction("Documentation");
}

// ── status bar ────────────────────────────────────────────────────────────────
void MainWindow::create_status_bar()
{
    status_label_ = new QLabel("◈  SYSTEM  IDLE");
    statusBar()->addWidget(status_label_);
    statusBar()->addPermanentWidget(new QLabel("BRAINLLM v4.0  |  Qt " QT_VERSION_STR));
}

// ── metrics update ────────────────────────────────────────────────────────────
void MainWindow::update_metrics()
{
    auto m = llm_engine_->get_metrics();
    float conf = llm_engine_->get_confidence();

    lbl_acc_->setText(    QString("%1%").arg(static_cast<int>(m.accuracy * 100)));
    lbl_conf_->setText(   QString("%1%").arg(static_cast<int>(conf       * 100)));
    lbl_cpu_->setText(    QString("%1%").arg(static_cast<int>(m.cpu_usage* 100)));
    lbl_tokens_->setText( QString::number(m.tokens_processed));
    lbl_state_->setText(  "IDLE");

    progress_bar_->setValue(static_cast<int>(m.accuracy * 100));
    viz_widget_->set_metrics(m);
}

// ── button slots ──────────────────────────────────────────────────────────────
void MainWindow::on_process_button_clicked()
{
    status_label_->setText("◈  PROCESSING");
    const QString input = editor_widget_->get_text().trimmed();
    if (input.isEmpty()) {
        console_widget_->write_line("[WARN]  No input text provided.");
        status_label_->setText("◈  IDLE");
        return;
    }
    const QString response = QString::fromStdString(
        llm_engine_->process_input(input.toStdString()));
    console_widget_->write_line("[IN]   " + input);
    console_widget_->write_line("[OUT]  " + response);
    status_label_->setText("◈  PROCESSED");
}

void MainWindow::on_generate_button_clicked()
{
    status_label_->setText("◈  GENERATING");
    const QString prompt = editor_widget_->get_text().trimmed();
    if (prompt.isEmpty()) {
        console_widget_->write_line("[WARN]  No prompt provided.");
        status_label_->setText("◈  IDLE");
        return;
    }
    const QString response = QString::fromStdString(
        llm_engine_->generate_response(prompt.toStdString(), 200));
    console_widget_->write_line("[PROMPT]  " + prompt);
    console_widget_->write_line("[RESP]    " + response);
    chat_widget_->append_message("YOU", prompt);
    chat_widget_->append_message("BRAINLLM", response);
    status_label_->setText("◈  GENERATED");

    // Speak the response through the AI voice
    voice_panel_->speak(response.toStdString());
}

void MainWindow::on_start_api_clicked()
{
    if (!api_server_->is_running()) {
        api_server_->set_llm_engine(llm_engine_);
        if (api_server_->start()) {
            start_api_button_->setText("⬡ STOP API");
            status_label_->setText("◈  API  ONLINE  :8080");
            console_widget_->write_line("[API]   Server running on http://localhost:8080");
        } else {
            console_widget_->write_line("[ERR]   Failed to start API server on :8080");
        }
    } else {
        api_server_->stop();
        start_api_button_->setText("⬡ API SRV");
        status_label_->setText("◈  API  OFFLINE");
        console_widget_->write_line("[API]   Server stopped.");
    }
}

void MainWindow::on_settings_clicked()
{
    ConfigManager cm;
    SettingsDialog dlg(cm, this);
    dlg.exec();
}

void MainWindow::on_train_clicked()
{
    status_label_->setText("◈  TRAINING");
    std::vector<std::string> data;
    const QString input = editor_widget_->get_text().trimmed();
    if (!input.isEmpty()) {
        for (const auto& line : input.split('\n', Qt::SkipEmptyParts))
            data.push_back(line.trimmed().toStdString());
    }
    if (data.empty())
        data = {"Hello", "World", "AI", "Neural", "Learning"};

    llm_engine_->train(data);
    console_widget_->write_line(
        QString("[TRAIN]  Completed — %1 samples").arg(static_cast<int>(data.size())));
    status_label_->setText("◈  TRAINING  COMPLETE");
}

void MainWindow::on_reset_clicked()
{
    llm_engine_->reset();
    console_widget_->write_line("[RESET]  Brain state cleared.");
    status_label_->setText("◈  RESET");
}

void MainWindow::on_memory_clear_clicked()
{
    llm_engine_->reset();
    console_widget_->write_line("[MEM]  Memory and context purged.");
    status_label_->setText("◈  MEMORY  CLEARED");
}

void MainWindow::on_voice_input(const QString& transcript)
{
    if (transcript.isEmpty()) return;
    console_widget_->write_line("[VOICE]  Input: " + transcript);
    chat_widget_->append_message("YOU", transcript);
    status_label_->setText("◈  VOICE  INPUT  RECEIVED");

    if (!llm_engine_) return;
    const QString response = QString::fromStdString(
        llm_engine_->generate_response(transcript.toStdString(), 200));
    chat_widget_->append_message("BRAINLLM", response);
    console_widget_->write_line("[VOICE]  Response: " + response);

    // Speak the response
    voice_panel_->speak(response.toStdString());
    status_label_->setText("◈  IDLE");
}

} // namespace BrainLLM
