#include "editor_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace BrainLLM {

EditorWidget::EditorWidget(QWidget* parent)
    : QWidget(parent) {
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    // Search box
    QHBoxLayout* search_layout = new QHBoxLayout();
    search_box_ = new QLineEdit();
    search_box_->setPlaceholderText("Search...");
    search_button_ = new QPushButton("Search");
    
    search_layout->addWidget(search_box_);
    search_layout->addWidget(search_button_);
    layout->addLayout(search_layout);
    
    // Editor
    editor_ = new QPlainTextEdit();
    editor_->setPlaceholderText("Enter input here...");
    layout->addWidget(editor_);
}

QString EditorWidget::get_text() const {
    return editor_->toPlainText();
}

void EditorWidget::set_text(const QString& text) {
    editor_->setPlainText(text);
}

void EditorWidget::clear() {
    editor_->clear();
}

} // namespace BrainLLM
