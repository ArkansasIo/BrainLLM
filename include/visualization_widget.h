#pragma once

#include <QWidget>
#include <QTimer>
#include <QVector>
#include <QPointF>
#include <memory>
#include "llm_engine.h"

namespace BrainLLM {

// =====================================================
// Neural Sphere Visualization — JARVIS / Ultron style
// Renders an animated glowing teal neural constellation
// =====================================================
class VisualizationWidget : public QWidget {
    Q_OBJECT

public:
    explicit VisualizationWidget(QWidget* parent = nullptr);
    ~VisualizationWidget() = default;

    void set_metrics(const BrainMetrics& metrics);
    void update_visualization();

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void tick();

private:
    struct Node {
        QPointF pos;        // 2D projected position
        float  phi;         // longitude on sphere
        float  theta;       // latitude on sphere
        float  pulse;       // individual glow phase
        float  size;
    };

    struct Edge { int a, b; };

    void build_sphere();
    void project_nodes();
    QPointF project(float x, float y, float z) const;
    void draw_background(QPainter& p);
    void draw_scan_lines(QPainter& p);
    void draw_rings(QPainter& p);
    void draw_edges(QPainter& p);
    void draw_nodes(QPainter& p);
    void draw_data_labels(QPainter& p);
    void draw_hud_overlay(QPainter& p);

    BrainMetrics current_metrics_;
    QTimer*       anim_timer_;
    float         rotation_angle_;     // degrees, incremented each tick
    float         pulse_phase_;        // global pulse 0..2π
    float         scan_y_;             // scanning line y position

    QVector<Node> nodes_;
    QVector<Edge> edges_;
};

} // namespace BrainLLM
