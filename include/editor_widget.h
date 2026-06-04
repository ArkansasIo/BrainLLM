#pragma once

#include <QWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QLineEdit>

namespace BrainLLM {

class EditorWidget : public QWidget {
    Q_OBJECT

public:
    EditorWidget(QWidget* parent = nullptr);
    ~EditorWidget() = default;
    
    QString get_text() const;
    void set_text(const QString& text);
    void clear();
    
private:
    QPlainTextEdit* editor_;
    QLineEdit* search_box_;
    QPushButton* search_button_;
};

} // namespace BrainLLM
