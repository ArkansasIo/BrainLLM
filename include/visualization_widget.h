#pragma once

#include <QWidget>
#include <memory>
#include "llm_engine.h"

namespace BrainLLM {

class VisualizationWidget : public QWidget {
    Q_OBJECT

public:
    VisualizationWidget(QWidget* parent = nullptr);
    ~VisualizationWidget() = default;
    
    void set_metrics(const BrainMetrics& metrics);
    void update_visualization();
    
protected:
    void paintEvent(QPaintEvent* event) override;
    
private:
    BrainMetrics current_metrics_;
};

} // namespace BrainLLM
