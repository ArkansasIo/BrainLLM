#include "editor_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

namespace BrainLLM {

EditorWidget::EditorWidget(QWidget* parent)
    : QWidget(parent)
{
    setStyleSheet(
        "EditorWidget {"
        "  background: #020d12;"
        "  border: 1px solid #1a5040;"
        "}");

    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // Header
    QWidget* header = new QWidget;
    header->setFixedHeight(32);
    header->setStyleSheet("background: #030f16; border-bottom: 1px solid #1a5040;");
    QHBoxLayout* hl = new QHBoxLayout(header);
    hl->setContentsMargins(10, 0, 10, 0);

    QLabel* title = new QLabel("◈  INPUT  TERMINAL");
    title->setStyleSheet(
        "color: #40ffdc; font-family: Consolas; font-size: 9px;"
        "letter-spacing: 3px; font-weight: bold; background: transparent;");
    hl->addWidget(title);
    hl->addStretch();

    search_box_ = new QLineEdit;
    search_box_->setPlaceholderText("search...");
    search_box_->setFixedWidth(110);
    search_box_->setFixedHeight(20);
    search_box_->setStyleSheet(
        "background: #020d12; color: #60c8a8;"
        "font-family: Consolas; font-size: 9px;"
        "border: 1px solid #1a5040; padding: 0 4px;");
    hl->addWidget(search_box_);

    search_button_ = new QPushButton("GO");
    search_button_->setFixedSize(28, 20);
    search_button_->setStyleSheet(
        "QPushButton {"
        "  color: #40ffdc; background: transparent;"
        "  font-family: Consolas; font-size: 8px; font-weight: bold;"
        "  border: 1px solid #1a5040; border-radius: 2px; }"
        "QPushButton:hover { border-color: #40a080; color: #80ffdc; }");
    hl->addWidget(search_button_);
    root->addWidget(header);

    // Editor
    editor_ = new QPlainTextEdit;
    editor_->setPlaceholderText("// Enter prompt, code, or training data here...");
    editor_->setStyleSheet(
        "QPlainTextEdit {"
        "  background: #020d12;"
        "  color: #90d8c0;"
        "  font-family: Consolas;"
        "  font-size: 11px;"
        "  border: none;"
        "  padding: 8px;"
        "  selection-background-color: #0d3028;"
        "}");
    root->addWidget(editor_, 1);
}

QString EditorWidget::get_text() const
{
    return editor_->toPlainText();
}

void EditorWidget::set_text(const QString& text)
{
    editor_->setPlainText(text);
}

void EditorWidget::clear()
{
    editor_->clear();
}

} // namespace BrainLLM
