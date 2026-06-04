#include "console_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>
#include <QScrollBar>

namespace BrainLLM {

ConsoleWidget::ConsoleWidget(QWidget* parent)
    : QWidget(parent)
{
    setStyleSheet(
        "ConsoleWidget {"
        "  background: #020d12;"
        "  border: 1px solid #1a5040;"
        "}");

    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // Header
    QWidget* header = new QWidget;
    header->setFixedHeight(32);
    header->setStyleSheet(
        "background: #030f16; border-bottom: 1px solid #1a5040;");
    QHBoxLayout* hl = new QHBoxLayout(header);
    hl->setContentsMargins(10, 0, 10, 0);

    QLabel* title = new QLabel("◈  SYSTEM  LOG");
    title->setStyleSheet(
        "color: #40ffdc; font-family: Consolas; font-size: 9px;"
        "letter-spacing: 3px; font-weight: bold; background: transparent;");
    hl->addWidget(title);
    hl->addStretch();

    clear_button_ = new QPushButton("CLEAR");
    clear_button_->setFixedSize(48, 20);
    clear_button_->setStyleSheet(
        "QPushButton {"
        "  color: #407060; background: transparent;"
        "  font-family: Consolas; font-size: 8px; font-weight: bold;"
        "  border: 1px solid #1a4035; border-radius: 2px; }"
        "QPushButton:hover { color: #80ffdc; border-color: #40a080; }"
        "QPushButton:pressed { background: #0a2a22; }");
    hl->addWidget(clear_button_);
    root->addWidget(header);

    // Console output
    console_ = new QPlainTextEdit;
    console_->setReadOnly(true);
    console_->setStyleSheet(
        "QPlainTextEdit {"
        "  background: #020d12;"
        "  color: #60c8a8;"
        "  font-family: Consolas;"
        "  font-size: 10px;"
        "  border: none;"
        "  padding: 6px;"
        "  selection-background-color: #0d3028;"
        "}");
    root->addWidget(console_, 1);

    connect(clear_button_, &QPushButton::clicked, this, &ConsoleWidget::on_clear_clicked);

    write_line("[BOOT]  BrainLLM kernel loaded");
    write_line("[INIT]  Neural engine ready");
    write_line("[NET]   Wolfram Alpha client standby");
    write_line("[LANG]  English / multilingual modules online");
}

void ConsoleWidget::write_line(const QString& text)
{
    QString ts = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    // Colour-code by prefix
    QString line;
    if (text.startsWith("[API]") || text.startsWith("[NET]")) {
        line = QString("<span style='color:#40a8c8;'>%1  %2</span>").arg(ts, text.toHtmlEscaped());
    } else if (text.startsWith("[ERR]") || text.startsWith("[WARN]")) {
        line = QString("<span style='color:#e06040;'>%1  %2</span>").arg(ts, text.toHtmlEscaped());
    } else if (text.startsWith("[BOOT]") || text.startsWith("[INIT]")) {
        line = QString("<span style='color:#40ffdc;'>%1  %2</span>").arg(ts, text.toHtmlEscaped());
    } else {
        line = QString("<span style='color:#60c8a8;'>%1  %2</span>").arg(ts, text.toHtmlEscaped());
    }

    // QPlainTextEdit doesn't render HTML; use appendHtml on a plain text doc workaround:
    // Just format as plain text with prefix colour via stylesheet override
    QString plain = ts + "  " + text;
    console_->appendPlainText(plain);
    console_->verticalScrollBar()->setValue(console_->verticalScrollBar()->maximum());
}

void ConsoleWidget::clear()
{
    console_->clear();
}

void ConsoleWidget::on_clear_clicked()
{
    clear();
}

} // namespace BrainLLM
