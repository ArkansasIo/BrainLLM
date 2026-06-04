#pragma once

#include <QWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QLabel>
#include <memory>
#include "llm_engine.h"

namespace BrainLLM {

class ChatWidget : public QWidget {
    Q_OBJECT

public:
    explicit ChatWidget(QWidget* parent = nullptr);
    ~ChatWidget() = default;

    void set_llm_engine(std::shared_ptr<LLMEngine> llm_engine);
    void append_message(const QString& speaker, const QString& message);
    void clear();

private slots:
    void on_send_clicked();
    void on_clear_clicked();

private:
    std::shared_ptr<LLMEngine> llm_engine_;
    QLabel* title_label_;
    QPlainTextEdit* transcript_;
    QPlainTextEdit* input_;
    QPushButton* send_button_;
    QPushButton* clear_button_;
};

} // namespace BrainLLM
