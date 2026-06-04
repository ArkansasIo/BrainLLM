#include "chat_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QTextCursor>

namespace BrainLLM {

ChatWidget::ChatWidget(QWidget* parent)
    : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(8);

    title_label_ = new QLabel("ChatGPT-style BrainLLM Chat");
    QFont title_font = title_label_->font();
    title_font.setPointSize(title_font.pointSize() + 3);
    title_font.setBold(true);
    title_label_->setFont(title_font);
    layout->addWidget(title_label_);

    transcript_ = new QPlainTextEdit();
    transcript_->setReadOnly(true);
    transcript_->setPlaceholderText("Chat output will appear here...");
    transcript_->setMinimumHeight(360);
    layout->addWidget(transcript_, 1);

    input_ = new QPlainTextEdit();
    input_->setPlaceholderText("Type a prompt, then click Send...");
    input_->setMaximumHeight(110);
    layout->addWidget(input_);

    QHBoxLayout* button_layout = new QHBoxLayout();
    button_layout->addStretch();

    clear_button_ = new QPushButton("Clear Chat");
    send_button_ = new QPushButton("Send");
    send_button_->setDefault(true);

    button_layout->addWidget(clear_button_);
    button_layout->addWidget(send_button_);
    layout->addLayout(button_layout);

    connect(send_button_, &QPushButton::clicked, this, &ChatWidget::on_send_clicked);
    connect(clear_button_, &QPushButton::clicked, this, &ChatWidget::on_clear_clicked);
}

void ChatWidget::set_llm_engine(std::shared_ptr<LLMEngine> llm_engine) {
    llm_engine_ = std::move(llm_engine);
}

void ChatWidget::append_message(const QString& speaker, const QString& message) {
    if (!transcript_->toPlainText().isEmpty()) {
        transcript_->appendPlainText("");
    }
    transcript_->appendPlainText(speaker + ":");
    transcript_->appendPlainText(message);
    transcript_->moveCursor(QTextCursor::End);
}

void ChatWidget::clear() {
    transcript_->clear();
}

void ChatWidget::on_send_clicked() {
    const QString prompt = input_->toPlainText().trimmed();
    if (prompt.isEmpty()) {
        append_message("System", "Enter a prompt before sending.");
        return;
    }

    append_message("You", prompt);
    input_->clear();

    if (!llm_engine_) {
        append_message("BrainLLM", "LLM engine is not connected.");
        return;
    }

    const QString response = QString::fromStdString(
        llm_engine_->generate_response(prompt.toStdString(), 180));
    append_message("BrainLLM", response);
}

void ChatWidget::on_clear_clicked() {
    clear();
}

} // namespace BrainLLM
