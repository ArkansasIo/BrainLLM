#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
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
    void animate_typing();

private:
    void apply_hud_style();

    std::shared_ptr<LLMEngine> llm_engine_;
    QTextEdit*   transcript_;
    QLineEdit*   input_;
    QPushButton* send_button_;
    QPushButton* clear_button_;
    QLabel*      status_label_;
    QTimer*      typing_timer_;
    int          typing_dots_;
};

} // namespace BrainLLM
