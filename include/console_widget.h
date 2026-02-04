#pragma once

#include <QWidget>
#include <QPlainTextEdit>
#include <QPushButton>

namespace BrainLLM {

class ConsoleWidget : public QWidget {
    Q_OBJECT

public:
    ConsoleWidget(QWidget* parent = nullptr);
    ~ConsoleWidget() = default;
    
    void write_line(const QString& text);
    void clear();
    
public slots:
    void on_clear_clicked();
    
private:
    QPlainTextEdit* console_;
    QPushButton* clear_button_;
};

} // namespace BrainLLM
