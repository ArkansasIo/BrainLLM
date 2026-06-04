#include "chat_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>
#include <QScrollBar>

namespace BrainLLM {

ChatWidget::ChatWidget(QWidget* parent)
    : QWidget(parent),
      typing_timer_(new QTimer(this)),
      typing_dots_(0)
{
    apply_hud_style();

    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ── header bar ───────────────────────────────────────────────────────────
    QWidget* header = new QWidget;
    header->setFixedHeight(36);
    header->setStyleSheet(
        "background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "  stop:0 #051219, stop:1 #0a2030);"
        "border-bottom: 1px solid #1a5040;");
    QHBoxLayout* hl = new QHBoxLayout(header);
    hl->setContentsMargins(12, 0, 12, 0);

    QLabel* title = new QLabel("◈  BRAINLLM  INTERFACE  v4.0");
    title->setStyleSheet(
        "color: #40ffdc; font-family: Consolas; font-size: 10px;"
        "letter-spacing: 3px; font-weight: bold; background: transparent;");
    hl->addWidget(title);
    hl->addStretch();

    status_label_ = new QLabel("● ONLINE");
    status_label_->setStyleSheet(
        "color: #00ffc8; font-family: Consolas; font-size: 9px;"
        "letter-spacing: 2px; background: transparent;");
    hl->addWidget(status_label_);
    root->addWidget(header);

    // ── transcript ───────────────────────────────────────────────────────────
    transcript_ = new QTextEdit;
    transcript_->setReadOnly(true);
    transcript_->setStyleSheet(
        "background: #020d12;"
        "color: #b0e8d8;"
        "font-family: Consolas;"
        "font-size: 11px;"
        "border: none;"
        "padding: 10px;");
    root->addWidget(transcript_, 1);

    // ── divider ───────────────────────────────────────────────────────────────
    QWidget* divider = new QWidget;
    divider->setFixedHeight(1);
    divider->setStyleSheet("background: #1a5040;");
    root->addWidget(divider);

    // ── input row ─────────────────────────────────────────────────────────────
    QWidget* input_row = new QWidget;
    input_row->setStyleSheet("background: #030f16; border-top: 1px solid #0e4035;");
    QHBoxLayout* il = new QHBoxLayout(input_row);
    il->setContentsMargins(10, 8, 10, 8);
    il->setSpacing(8);

    QLabel* prompt_label = new QLabel("▶");
    prompt_label->setStyleSheet(
        "color: #40ffdc; font-size: 12px; background: transparent;");
    il->addWidget(prompt_label);

    input_ = new QLineEdit;
    input_->setPlaceholderText("Enter command or query...");
    input_->setStyleSheet(
        "background: transparent;"
        "color: #80f0d8;"
        "font-family: Consolas;"
        "font-size: 11px;"
        "border: none;"
        "selection-background-color: #1a6050;");
    il->addWidget(input_, 1);

    clear_button_ = new QPushButton("CLR");
    clear_button_->setFixedSize(40, 26);
    clear_button_->setStyleSheet(
        "QPushButton {"
        "  color: #407060; background: transparent;"
        "  font-family: Consolas; font-size: 9px; font-weight: bold;"
        "  border: 1px solid #1a4035; border-radius: 3px; }"
        "QPushButton:hover {"
        "  color: #80ffdc; border-color: #40a080; }"
        "QPushButton:pressed { background: #0a2a22; }");
    il->addWidget(clear_button_);

    send_button_ = new QPushButton("SEND");
    send_button_->setFixedSize(55, 26);
    send_button_->setStyleSheet(
        "QPushButton {"
        "  color: #00ffcc; background: #042018;"
        "  font-family: Consolas; font-size: 9px; font-weight: bold;"
        "  border: 1px solid #00ffcc; border-radius: 3px; }"
        "QPushButton:hover {"
        "  background: #073525; }"
        "QPushButton:pressed { background: #0a4535; }");
    il->addWidget(send_button_);
    root->addWidget(input_row);

    // ── connections ──────────────────────────────────────────────────────────
    connect(send_button_,  &QPushButton::clicked, this, &ChatWidget::on_send_clicked);
    connect(clear_button_, &QPushButton::clicked, this, &ChatWidget::on_clear_clicked);
    connect(input_, &QLineEdit::returnPressed,    this, &ChatWidget::on_send_clicked);
    connect(typing_timer_, &QTimer::timeout,      this, &ChatWidget::animate_typing);

    // Welcome message
    append_message("SYSTEM",
        "BrainLLM Neural Interface online.\n"
        "Wolfram Alpha, AirLLM, and cognitive modules linked.\n"
        "Type 'wolfram: &lt;query&gt;' for computational queries.");
}

void ChatWidget::apply_hud_style()
{
    setStyleSheet(
        "ChatWidget {"
        "  background: #020d12;"
        "  border: 1px solid #1a5040;"
        "}");
}

void ChatWidget::set_llm_engine(std::shared_ptr<LLMEngine> llm_engine)
{
    llm_engine_ = std::move(llm_engine);
}

void ChatWidget::append_message(const QString& speaker, const QString& message)
{
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss");

    QString colour;
    QString prefix;
    if (speaker == "YOU" || speaker == "You") {
        colour  = "#40ffdc";
        prefix  = "▶ YOU";
    } else if (speaker == "SYSTEM") {
        colour  = "#408878";
        prefix  = "◈ SYSTEM";
    } else {
        colour  = "#80f0c8";
        prefix  = "◆ BRAINLLM";
    }

    transcript_->append(
        QString("<span style='color:%1; font-family:Consolas; font-size:10px;"
                "letter-spacing:1px;'>%2</span>"
                "<span style='color:#204030; font-size:9px;'>  %3</span>")
            .arg(colour).arg(prefix).arg(timestamp));

    transcript_->append(
        QString("<span style='color:#b0e8d8; font-family:Consolas; font-size:11px;"
                "line-height:1.5;'>%1</span>")
            .arg(message.toHtmlEscaped().replace("\n", "<br>")));

    transcript_->append("<span style='color:#102018;'>─────────────────────────────</span>");
    transcript_->verticalScrollBar()->setValue(
        transcript_->verticalScrollBar()->maximum());
}

void ChatWidget::clear()
{
    transcript_->clear();
}

void ChatWidget::on_send_clicked()
{
    const QString prompt = input_->text().trimmed();
    if (prompt.isEmpty()) return;

    append_message("YOU", prompt);
    input_->clear();

    if (!llm_engine_) {
        append_message("SYSTEM", "LLM engine not connected.");
        return;
    }

    // Show typing indicator
    status_label_->setText("● PROCESSING");
    status_label_->setStyleSheet(
        "color: #ffcc00; font-family: Consolas; font-size: 9px;"
        "letter-spacing: 2px; background: transparent;");
    typing_dots_ = 0;
    typing_timer_->start(400);

    const QString response = QString::fromStdString(
        llm_engine_->generate_response(prompt.toStdString(), 200));

    typing_timer_->stop();
    append_message("BRAINLLM", response);

    status_label_->setText("● ONLINE");
    status_label_->setStyleSheet(
        "color: #00ffc8; font-family: Consolas; font-size: 9px;"
        "letter-spacing: 2px; background: transparent;");
}

void ChatWidget::on_clear_clicked()
{
    clear();
}

void ChatWidget::animate_typing()
{
    QString dots(typing_dots_ % 4, '.');
    status_label_->setText("● COMPUTING" + dots);
    ++typing_dots_;
}

} // namespace BrainLLM
