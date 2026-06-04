#include "visualization_widget.h"
#include <QPainter>
#include <QBrush>
#include <QPen>

namespace BrainLLM {

VisualizationWidget::VisualizationWidget(QWidget* parent)
    : QWidget(parent) {
    setMinimumHeight(300);
    setStyleSheet("background-color: #1e1e1e; border: 1px solid #333;");
}

void VisualizationWidget::set_metrics(const BrainMetrics& metrics) {
    current_metrics_ = metrics;
    update();
}

void VisualizationWidget::update_visualization() {
    update();
}

void VisualizationWidget::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw background
    painter.fillRect(rect(), QColor(30, 30, 30));
    
    // Draw neural network visualization
    painter.setPen(QPen(QColor(100, 200, 255), 2));
    painter.setBrush(QBrush(QColor(50, 150, 255)));
    
    int num_neurons = 8;
    int spacing_x = width() / (num_neurons + 1);
    int spacing_y = height() / 4;
    
    // Draw neurons
    for (int layer = 0; layer < 4; ++layer) {
        for (int neuron = 0; neuron < num_neurons; ++neuron) {
            int x = (neuron + 1) * spacing_x;
            int y = (layer + 1) * spacing_y;
            
            float intensity = (layer * num_neurons + neuron) % 100 / 100.0f;
            int color_val = 50 + static_cast<int>(intensity * 205);
            
            painter.setBrush(QBrush(QColor(color_val, color_val + 50, 255)));
            painter.drawEllipse(x - 5, y - 5, 10, 10);
        }
    }
    
    // Draw title
    painter.setPen(QPen(QColor(200, 200, 200), 1));
    painter.drawText(10, 20, "Neural Network Visualization");
    
    // Draw metrics
    painter.setPen(QPen(QColor(150, 255, 150), 1));
    painter.drawText(10, height() - 10, 
                     QString("CPU: %1% | Accuracy: %2% | Tokens: %3")
                     .arg(static_cast<int>(current_metrics_.cpu_usage * 100))
                     .arg(static_cast<int>(current_metrics_.accuracy * 100))
                     .arg(current_metrics_.tokens_processed));
}

} // namespace BrainLLM
