#include "visualization_widget.h"
#include <QPainter>
#include <QPainterPath>
#include <QRadialGradient>
#include <QLinearGradient>
#include <QConicalGradient>
#include <QFontDatabase>
#include <QtMath>
#include <cmath>
#include <cstdlib>

namespace BrainLLM {

// ── colour palette ──────────────────────────────────────────────────────────
static const QColor C_BG_DARK    (  5,  18,  25);
static const QColor C_BG_MID     (  8,  32,  42);
static const QColor C_TEAL_BRIGHT( 80, 255, 220);
static const QColor C_TEAL_MID   ( 40, 180, 160);
static const QColor C_TEAL_DIM   ( 20,  90,  80);
static const QColor C_CYAN       (  0, 230, 255);
static const QColor C_WHITE_DIM  (180, 230, 225);
static const QColor C_EDGE       ( 50, 170, 150, 120);
static const QColor C_RING       ( 60, 200, 190,  60);
static const QColor C_SCAN       (  0, 255, 200,  35);

// ── construction ────────────────────────────────────────────────────────────
VisualizationWidget::VisualizationWidget(QWidget* parent)
    : QWidget(parent),
      anim_timer_(new QTimer(this)),
      rotation_angle_(0.f),
      pulse_phase_(0.f),
      scan_y_(0.f)
{
    setMinimumSize(340, 340);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setStyleSheet("background: transparent;");

    build_sphere();

    connect(anim_timer_, &QTimer::timeout, this, &VisualizationWidget::tick);
    anim_timer_->start(30);   // ~33 fps
}

// ── sphere geometry ─────────────────────────────────────────────────────────
void VisualizationWidget::build_sphere()
{
    nodes_.clear();
    edges_.clear();

    // Fibonacci-sphere distribution for ~60 nodes
    const int N = 60;
    const float golden = (1.f + std::sqrt(5.f)) / 2.f;

    for (int i = 0; i < N; ++i) {
        float theta = std::acos(1.f - 2.f * (i + 0.5f) / N);   // latitude
        float phi   = 2.f * M_PI * i / golden;                   // longitude
        Node nd;
        nd.theta = theta;
        nd.phi   = phi;
        nd.pulse = static_cast<float>(std::rand() % 628) / 100.f;
        nd.size  = 2.5f + static_cast<float>(std::rand() % 30) / 10.f;
        nd.pos   = {0, 0};
        nodes_.append(nd);
    }

    // Short-range edges (constellation style)
    for (int i = 0; i < N; ++i) {
        for (int j = i + 1; j < N; ++j) {
            float dphi   = nodes_[i].phi   - nodes_[j].phi;
            float dtheta = nodes_[i].theta - nodes_[j].theta;
            float dist   = std::sqrt(dphi * dphi + dtheta * dtheta);
            if (dist < 0.55f) {
                edges_.append({i, j});
            }
        }
    }
}

// ── animation tick ──────────────────────────────────────────────────────────
void VisualizationWidget::tick()
{
    rotation_angle_ += 0.35f;
    if (rotation_angle_ >= 360.f) rotation_angle_ -= 360.f;

    pulse_phase_ += 0.06f;
    if (pulse_phase_ > 2.f * M_PI) pulse_phase_ -= 2.f * M_PI;

    scan_y_ += 2.5f;
    if (scan_y_ > height()) scan_y_ = -40.f;

    project_nodes();
    update();
}

// ── 3-D → 2-D projection ────────────────────────────────────────────────────
QPointF VisualizationWidget::project(float x, float y, float z) const
{
    float cx = width()  / 2.f;
    float cy = height() / 2.f;
    float r  = std::min(cx, cy) * 0.42f;

    // Perspective
    float fov    = 3.5f;
    float scale  = fov / (fov + z);
    return { cx + x * r * scale, cy + y * r * scale };
}

void VisualizationWidget::project_nodes()
{
    float rad = qDegreesToRadians(rotation_angle_);

    for (auto& nd : nodes_) {
        // Sphere → Cartesian
        float sx = std::sin(nd.theta) * std::cos(nd.phi);
        float sy = std::sin(nd.theta) * std::sin(nd.phi);
        float sz = std::cos(nd.theta);

        // Rotate around Y axis
        float rx = sx * std::cos(rad) + sz * std::sin(rad);
        float ry = sy;
        float rz = -sx * std::sin(rad) + sz * std::cos(rad);

        // Slight tilt around X axis
        float tilt = 0.25f;
        float fx = rx;
        float fy = ry * std::cos(tilt) - rz * std::sin(tilt);
        float fz = ry * std::sin(tilt) + rz * std::cos(tilt);

        nd.pos = project(fx, fy, fz);
    }
}

// ── set_metrics / update_visualization ──────────────────────────────────────
void VisualizationWidget::set_metrics(const BrainMetrics& m)
{
    current_metrics_ = m;
    update();
}

void VisualizationWidget::update_visualization() { update(); }

// ── paint ────────────────────────────────────────────────────────────────────
void VisualizationWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);

    draw_background(p);
    draw_scan_lines(p);
    draw_rings(p);
    draw_edges(p);
    draw_nodes(p);
    draw_data_labels(p);
    draw_hud_overlay(p);
}

// ── background ───────────────────────────────────────────────────────────────
void VisualizationWidget::draw_background(QPainter& p)
{
    // Deep teal gradient
    QRadialGradient bg(width() / 2, height() / 2, std::max(width(), height()) * 0.65f);
    bg.setColorAt(0.0, C_BG_MID);
    bg.setColorAt(1.0, C_BG_DARK);
    p.fillRect(rect(), bg);

    // Faint star-field
    p.setPen(QPen(QColor(160, 220, 210, 80), 1));
    std::srand(42);
    for (int i = 0; i < 80; ++i) {
        int sx = std::rand() % width();
        int sy = std::rand() % height();
        float br = static_cast<float>(std::rand() % 100) / 100.f;
        float phase = std::fmod(pulse_phase_ + br * 3.f, 2.f * M_PI);
        int alpha = 40 + static_cast<int>(40.f * std::sin(phase));
        p.setPen(QPen(QColor(160, 230, 210, alpha), 1.2f));
        p.drawPoint(sx, sy);
    }
}

// ── horizontal scan line sweeping down ───────────────────────────────────────
void VisualizationWidget::draw_scan_lines(QPainter& p)
{
    for (int dy = 0; dy < 6; ++dy) {
        int y = static_cast<int>(scan_y_) + dy * 6;
        if (y < 0 || y >= height()) continue;
        int alpha = 35 - dy * 5;
        if (alpha < 0) continue;
        p.setPen(QPen(QColor(0, 255, 200, alpha), 1));
        p.drawLine(0, y, width(), y);
    }
}

// ── concentric pulsing rings ─────────────────────────────────────────────────
void VisualizationWidget::draw_rings(QPainter& p)
{
    float cx = width()  / 2.f;
    float cy = height() / 2.f;
    float base_r = std::min(cx, cy) * 0.48f;

    for (int ri = 0; ri < 4; ++ri) {
        float phase_offset = ri * 0.5f;
        float pulse = std::sin(pulse_phase_ + phase_offset);
        float r = base_r * (0.88f + 0.12f * pulse) + ri * 12.f;
        int alpha = 55 - ri * 10 + static_cast<int>(20.f * pulse);
        if (alpha < 0) alpha = 0;

        QPen pen(QColor(60, 210, 190, alpha), 1.2f - ri * 0.2f);
        pen.setStyle(ri % 2 == 0 ? Qt::SolidLine : Qt::DotLine);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(QPointF(cx, cy), r, r * 0.38f);
    }

    // Spiral arc (the dramatic sweep from the image)
    QPainterPath arc;
    float sr = base_r * 1.22f;
    for (int deg = 0; deg < 320; ++deg) {
        float a = qDegreesToRadians(static_cast<float>(deg + rotation_angle_ * 0.5f));
        float fade = static_cast<float>(deg) / 320.f;
        float rx = cx + sr * fade * std::cos(a);
        float ry = cy + sr * 0.42f * fade * std::sin(a);
        if (deg == 0) arc.moveTo(rx, ry);
        else          arc.lineTo(rx, ry);
    }
    QPen spiral_pen(C_TEAL_BRIGHT, 1.8f);
    spiral_pen.setCapStyle(Qt::RoundCap);
    p.setPen(spiral_pen);
    p.setOpacity(0.6 + 0.25 * std::sin(pulse_phase_));
    p.drawPath(arc);
    p.setOpacity(1.0);
}

// ── constellation edges ───────────────────────────────────────────────────────
void VisualizationWidget::draw_edges(QPainter& p)
{
    float rad = qDegreesToRadians(rotation_angle_);
    float cx = width()  / 2.f;
    float cy = height() / 2.f;

    for (const auto& e : edges_) {
        // Depth-based alpha: nodes further back are dimmer
        auto& na = nodes_[e.a];
        auto& nb = nodes_[e.b];

        float za = std::sin(na.theta) * std::sin(na.phi + rad);
        float zb = std::sin(nb.theta) * std::sin(nb.phi + rad);
        float depth = (za + zb) / 2.f;       // -1 .. +1
        int alpha = static_cast<int>(30.f + 90.f * ((depth + 1.f) / 2.f));

        p.setPen(QPen(QColor(50, 175, 155, alpha), 0.8f));
        p.drawLine(na.pos, nb.pos);
    }
    Q_UNUSED(cx); Q_UNUSED(cy);
}

// ── nodes (glowing dots) ─────────────────────────────────────────────────────
void VisualizationWidget::draw_nodes(QPainter& p)
{
    float rad = qDegreesToRadians(rotation_angle_);

    for (const auto& nd : nodes_) {
        // Depth for front/back distinction
        float z = std::sin(nd.theta) * std::sin(nd.phi + rad);
        float visibility = (z + 1.f) / 2.f;   // 0 (back) .. 1 (front)

        float glow = std::sin(pulse_phase_ + nd.pulse) * 0.5f + 0.5f;
        float r    = nd.size * (0.6f + 0.4f * visibility);

        // Outer glow
        QRadialGradient grd(nd.pos, r * 3.5f);
        grd.setColorAt(0.0, QColor(80, 255, 220, static_cast<int>(120 * visibility * glow)));
        grd.setColorAt(1.0, QColor(0,   0,   0,  0));
        p.setBrush(grd);
        p.setPen(Qt::NoPen);
        p.drawEllipse(nd.pos, r * 3.5, r * 3.5);

        // Core dot
        QColor core = visibility > 0.6f ? C_TEAL_BRIGHT : C_TEAL_MID;
        core.setAlpha(static_cast<int>(180 * visibility));
        p.setBrush(core);
        p.setPen(QPen(C_CYAN.darker(150), 0.5f));
        p.drawEllipse(nd.pos, r, r);
    }
}

// ── floating data labels (holographic readout) ───────────────────────────────
void VisualizationWidget::draw_data_labels(QPainter& p)
{
    QFont f;
    f.setFamily("Consolas");
    f.setPointSizeF(7.0);
    p.setFont(f);

    // Pick 12 nodes spread around and draw data labels next to them
    const QStringList labels = {
        "1.384", "0.921", "∆0.003", "M.923",
        "KINESIS", "4.753", "0.100", "NODE:47",
        "SYNC", "0.024", "0.233", "0.481"
    };

    int shown = 0;
    for (int i = 0; i < nodes_.size() && shown < labels.size(); i += 5) {
        const auto& nd = nodes_[i];
        float z = std::sin(nd.theta) * std::sin(nd.phi + qDegreesToRadians(rotation_angle_));
        if (z < 0.1f) continue;  // only front-facing

        float alpha = 80.f + 100.f * ((z + 1.f) / 2.f);
        p.setPen(QColor(140, 220, 200, static_cast<int>(alpha)));
        p.drawText(nd.pos + QPointF(nd.size + 4, 4), labels[shown]);
        ++shown;
    }
}

// ── HUD overlay (metrics, title, status bar) ─────────────────────────────────
void VisualizationWidget::draw_hud_overlay(QPainter& p)
{
    // Corner brackets
    int bw = 18, bh = 18;
    QPen bracket_pen(C_TEAL_MID, 1.5f);
    p.setPen(bracket_pen);
    p.drawLine(4, 4,  4 + bw, 4);
    p.drawLine(4, 4,  4, 4 + bh);
    p.drawLine(width() - 4, 4,  width() - 4 - bw, 4);
    p.drawLine(width() - 4, 4,  width() - 4, 4 + bh);
    p.drawLine(4, height() - 4,  4 + bw, height() - 4);
    p.drawLine(4, height() - 4,  4, height() - 4 - bh);
    p.drawLine(width() - 4, height() - 4,  width() - 4 - bw, height() - 4);
    p.drawLine(width() - 4, height() - 4,  width() - 4, height() - 4 - bh);

    // Title
    QFont title_font("Consolas", 9);
    title_font.setBold(true);
    title_font.setLetterSpacing(QFont::AbsoluteSpacing, 2.5);
    p.setFont(title_font);
    p.setPen(C_TEAL_BRIGHT);
    p.drawText(QRect(0, 6, width(), 20), Qt::AlignHCenter, "BRAINLLM  NEURAL  CORE");

    // Pulsing dot on title
    float pulse = std::sin(pulse_phase_) * 0.5f + 0.5f;
    QRadialGradient dot_grd(width() - 22, 14, 6);
    dot_grd.setColorAt(0.0, QColor(0, 255, 180, static_cast<int>(200 * pulse)));
    dot_grd.setColorAt(1.0, QColor(0, 0, 0, 0));
    p.setBrush(dot_grd);
    p.setPen(Qt::NoPen);
    p.drawEllipse(width() - 28, 8, 12, 12);

    // Bottom metrics bar
    QFont metrics_font("Consolas", 7);
    p.setFont(metrics_font);
    p.setPen(QColor(100, 200, 180, 200));
    QString metrics_str = QString("CPU %1%  |  ACC %2%  |  TOKENS %3  |  CONF %4%")
        .arg(static_cast<int>(current_metrics_.cpu_usage   * 100))
        .arg(static_cast<int>(current_metrics_.accuracy    * 100))
        .arg(current_metrics_.tokens_processed)
        .arg(static_cast<int>(current_metrics_.accuracy    * 100));
    p.drawText(QRect(0, height() - 20, width(), 16),
               Qt::AlignHCenter | Qt::AlignVCenter, metrics_str);

    // Divider line
    p.setPen(QPen(C_TEAL_DIM, 1));
    p.drawLine(10, height() - 24, width() - 10, height() - 24);
}

} // namespace BrainLLM
