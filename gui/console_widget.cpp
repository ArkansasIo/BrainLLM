#include "console_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace BrainLLM {

ConsoleWidget::ConsoleWidget(QWidget* parent)
    : QWidget(parent) {
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    // Console display
    console_ = new QPlainTextEdit();
    console_->setReadOnly(true);
    console_->setPlaceholderText("Output will appear here...");
    layout->addWidget(console_);
    
    // Clear button
    QHBoxLayout* button_layout = new QHBoxLayout();
    clear_button_ = new QPushButton("Clear Console");
    button_layout->addStretch();
    button_layout->addWidget(clear_button_);
    layout->addLayout(button_layout);
    
    connect(clear_button_, &QPushButton::clicked, this, &ConsoleWidget::on_clear_clicked);
}

void ConsoleWidget::write_line(const QString& text) {
    console_->appendPlainText(text);
}

void ConsoleWidget::clear() {
    console_->clear();
}

void ConsoleWidget::on_clear_clicked() {
    clear();
}

} // namespace BrainLLM
