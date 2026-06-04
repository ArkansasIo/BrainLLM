#include "voice_panel.h"
#include "wake_word.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPainter>
#include <QPainterPath>
#include <QRadialGradient>
#include <QtMath>
#include <cmath>
#include <cstdlib>

namespace BrainLLM {

// ════════════════════════════════════════════════════════════════════════════
// VoiceWaveform
// ════════════════════════════════════════════════════════════════════════════
VoiceWaveform::VoiceWaveform(QWidget* parent)
    : QWidget(parent),
      anim_timer_(new QTimer(this)),
      active_(false),
      target_amp_(0.15f),
      phase_(0.f)
{
    setFixedHeight(48);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setStyleSheet("background: transparent;");

    // Initialise bars with low random baseline
    const int n = 32;
    bars_.resize(n);
    for (int i = 0; i < n; ++i)
        bars_[i] = 0.05f + static_cast<float>(std::rand() % 10) / 100.f;

    connect(anim_timer_, &QTimer::timeout, this, &VoiceWaveform::tick);
    anim_timer_->start(40);
}

void VoiceWaveform::set_active(bool active)
{
    active_ = active;
    target_amp_ = active ? 0.75f : 0.12f;
}

void VoiceWaveform::set_amplitude(float amp)
{
    target_amp_ = std::max(0.f, std::min(1.f, amp));
}

void VoiceWaveform::tick()
{
    phase_ += active_ ? 0.18f : 0.06f;
    if (phase_ > 2.f * M_PI) phase_ -= 2.f * M_PI;

    for (int i = 0; i < bars_.size(); ++i) {
        float wave  = std::sin(phase_ + i * 0.42f) * 0.5f + 0.5f;
        float noise = static_cast<float>(std::rand() % 100) / 100.f;
        float target = target_amp_ * (0.4f + 0.6f * wave) + noise * 0.05f;
        bars_[i] += (target - bars_[i]) * 0.22f;
    }
    update();
}

void VoiceWaveform::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int n   = bars_.size();
    const float bw = static_cast<float>(width()) / n;
    const float cy = height() / 2.f;

    for (int i = 0; i < n; ++i) {
        float h = bars_[i] * (height() - 4.f);
        h = std::max(2.f, h);

        // Colour gradient: brighter when taller
        int g = 180 + static_cast<int>(75.f * bars_[i]);
        QColor col(0, g, std::min(255, g + 30), 210);

        float x = i * bw + bw * 0.15f;
        float w = bw * 0.7f;
        p.fillRect(QRectF(x, cy - h / 2.f, w, h), col);
    }

    // Centre line
    p.setPen(QPen(QColor(0, 100, 80, 60), 1));
    p.drawLine(0, static_cast<int>(cy), width(), static_cast<int>(cy));
}

// ════════════════════════════════════════════════════════════════════════════
// VoicePanel
// ════════════════════════════════════════════════════════════════════════════
VoicePanel::VoicePanel(QWidget* parent)
    : QWidget(parent),
      wake_words_(default_wake_words()),
      rec_timer_(new QTimer(this))
{
    setStyleSheet(
        "VoicePanel {"
        "  background: #020d12;"
        "  border: 1px solid #1a5040;"
        "}");
    build_ui();

    connect(rec_timer_, &QTimer::timeout, this, &VoicePanel::recognition_finished);
}

void VoicePanel::set_llm_engine(std::shared_ptr<LLMEngine> engine)
{
    llm_engine_ = std::move(engine);
}

void VoicePanel::set_wake_words(const std::vector<std::string>& wake_words)
{
    wake_words_ = wake_words.empty() ? default_wake_words() : wake_words;
}

void VoicePanel::set_wake_word_required(bool required)
{
    require_wake_word_ = required;
}

// ── build_ui ─────────────────────────────────────────────────────────────────
void VoicePanel::build_ui()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ── Header ────────────────────────────────────────────────────────────────
    QWidget* header = new QWidget;
    header->setFixedHeight(34);
    header->setStyleSheet("background: #030f16; border-bottom: 1px solid #1a5040;");
    QHBoxLayout* hl = new QHBoxLayout(header);
    hl->setContentsMargins(10, 0, 10, 0);

    QLabel* title = new QLabel("◈  AI  VOICE  SYSTEM");
    title->setStyleSheet(
        "color: #40ffdc; font-family: Consolas; font-size: 9px;"
        "letter-spacing: 3px; font-weight: bold; background: transparent;");
    hl->addWidget(title);
    hl->addStretch();

    status_label_ = new QLabel("● READY");
    status_label_->setStyleSheet(
        "color: #00ffc8; font-family: Consolas; font-size: 9px;"
        "letter-spacing: 2px; background: transparent;");
    hl->addWidget(status_label_);
    root->addWidget(header);

    // ── Waveform ──────────────────────────────────────────────────────────────
    waveform_ = new VoiceWaveform;
    QWidget* wave_wrap = new QWidget;
    wave_wrap->setStyleSheet("background: #010810; border-bottom: 1px solid #0e3530;");
    QHBoxLayout* wl = new QHBoxLayout(wave_wrap);
    wl->setContentsMargins(6, 4, 6, 4);
    wl->addWidget(waveform_);
    root->addWidget(wave_wrap);

    // ── Voice name label ──────────────────────────────────────────────────────
    name_label_ = new QLabel("ARIA — AI Intelligence  ·  en-US  ·  pitch 195 Hz  ·  rate 1.05×");
    name_label_->setStyleSheet(
        "color: #408878; font-family: Consolas; font-size: 8px;"
        "letter-spacing: 1px; background: #010810; padding: 2px 8px;"
        "border-bottom: 1px solid #0e3530;");
    root->addWidget(name_label_);

    // ── Controls grid ─────────────────────────────────────────────────────────
    QWidget* ctrl = new QWidget;
    ctrl->setStyleSheet("background: #020d12;");
    QGridLayout* grid = new QGridLayout(ctrl);
    grid->setContentsMargins(10, 8, 10, 8);
    grid->setSpacing(6);

    auto make_label = [](const QString& t) {
        QLabel* l = new QLabel(t);
        l->setStyleSheet(
            "color: #306050; font-family: Consolas; font-size: 8px;"
            "letter-spacing: 1px; background: transparent;");
        return l;
    };

    auto make_slider = [](int min, int max, int val) {
        QSlider* s = new QSlider(Qt::Horizontal);
        s->setRange(min, max);
        s->setValue(val);
        s->setStyleSheet(
            "QSlider::groove:horizontal {"
            "  background: #0a2a22; height: 3px; border-radius: 1px; }"
            "QSlider::handle:horizontal {"
            "  background: #00ffcc; width: 10px; height: 10px;"
            "  border-radius: 5px; margin: -4px 0; }"
            "QSlider::sub-page:horizontal {"
            "  background: #00aa88; border-radius: 1px; }");
        return s;
    };

    auto make_val_label = [](const QString& t) {
        QLabel* l = new QLabel(t);
        l->setFixedWidth(38);
        l->setStyleSheet(
            "color: #40ffdc; font-family: Consolas; font-size: 9px; background: transparent;");
        l->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        return l;
    };

    // Row 0: Personality
    grid->addWidget(make_label("PERSONALITY"), 0, 0);
    personality_combo_ = new QComboBox;
    personality_combo_->addItems({"ARIA — AI Intelligence",
                                   "NOVA — Analytical",
                                   "SAGE — Authoritative",
                                   "ECHO — Gentle"});
    personality_combo_->setStyleSheet(
        "QComboBox {"
        "  background: #030f16; color: #80d8c0;"
        "  font-family: Consolas; font-size: 9px;"
        "  border: 1px solid #1a5040; padding: 2px 6px; }"
        "QComboBox::drop-down { border: none; width: 18px; }"
        "QComboBox QAbstractItemView {"
        "  background: #030f16; color: #80d8c0;"
        "  selection-background-color: #0d3028; border: 1px solid #1a5040; }");
    grid->addWidget(personality_combo_, 0, 1, 1, 3);

    // Row 1: Voice (installed SAPI voices)
    grid->addWidget(make_label("VOICE"), 1, 0);
    voice_combo_ = new QComboBox;
    voice_combo_->setStyleSheet(personality_combo_->styleSheet());
    voice_combo_->addItem("Auto (female preference)", "");
    grid->addWidget(voice_combo_, 1, 1, 1, 3);

    // Row 2: Pitch
    grid->addWidget(make_label("PITCH"), 2, 0);
    pitch_slider_ = make_slider(140, 260, 195);
    pitch_val_label_ = make_val_label("195 Hz");
    grid->addWidget(pitch_slider_, 2, 1, 1, 2);
    grid->addWidget(pitch_val_label_, 2, 3);

    // Row 3: Rate
    grid->addWidget(make_label("RATE"), 3, 0);
    rate_slider_ = make_slider(60, 150, 105);
    rate_val_label_ = make_val_label("1.05×");
    grid->addWidget(rate_slider_, 3, 1, 1, 2);
    grid->addWidget(rate_val_label_, 3, 3);

    // Row 4: Volume
    grid->addWidget(make_label("VOLUME"), 4, 0);
    volume_slider_ = make_slider(0, 100, 100);
    vol_val_label_ = make_val_label("100%");
    grid->addWidget(volume_slider_, 4, 1, 1, 2);
    grid->addWidget(vol_val_label_, 4, 3);

    root->addWidget(ctrl);

    // ── Button row ────────────────────────────────────────────────────────────
    QWidget* btn_bar = new QWidget;
    btn_bar->setStyleSheet("background: #030f16; border-top: 1px solid #1a5040;");
    QHBoxLayout* bl = new QHBoxLayout(btn_bar);
    bl->setContentsMargins(10, 6, 10, 6);
    bl->setSpacing(6);

    auto make_btn = [](const QString& t, const QString& col = "#00ffcc") {
        QPushButton* b = new QPushButton(t);
        b->setFixedHeight(26);
        b->setStyleSheet(
            QString("QPushButton {"
                    "  color:%1; background:transparent;"
                    "  font-family:Consolas; font-size:9px; font-weight:bold;"
                    "  letter-spacing:1px;"
                    "  border:1px solid %1; border-radius:2px; padding:0 8px;}"
                    "QPushButton:hover{background:rgba(0,255,200,0.07);}"
                    "QPushButton:pressed{background:rgba(0,255,200,0.15);}").arg(col));
        return b;
    };

    speak_button_   = make_btn("▶ SPEAK");
    mic_button_     = make_btn("◉ MIC",   "#ff6060");
    test_button_    = make_btn("◈ TEST",  "#60a0ff");
    auto_speak_btn_ = make_btn("⬡ AUTO",  "#80a090");
    auto_speak_btn_->setCheckable(true);

    bl->addWidget(speak_button_);
    bl->addWidget(mic_button_);
    bl->addWidget(test_button_);
    bl->addStretch();
    bl->addWidget(auto_speak_btn_);
    root->addWidget(btn_bar);

    // ── Connections ───────────────────────────────────────────────────────────
    connect(personality_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &VoicePanel::on_personality_changed);
    connect(voice_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &VoicePanel::on_voice_changed);
    connect(pitch_slider_,  &QSlider::valueChanged, this, &VoicePanel::on_pitch_changed);
    connect(rate_slider_,   &QSlider::valueChanged, this, &VoicePanel::on_rate_changed);
    connect(volume_slider_, &QSlider::valueChanged, this, &VoicePanel::on_volume_changed);
    connect(speak_button_,  &QPushButton::clicked,  this, &VoicePanel::on_speak_button_clicked);
    connect(mic_button_,    &QPushButton::clicked,  this, &VoicePanel::on_mic_button_clicked);
    connect(test_button_,   &QPushButton::clicked,  this, &VoicePanel::on_test_button_clicked);
    connect(auto_speak_btn_,&QPushButton::toggled,  this, &VoicePanel::on_speak_responses_toggled);
}

// ── Voice list ────────────────────────────────────────────────────────────────
void VoicePanel::refresh_voice_list()
{
    if (!voice_combo_) return;

    voice_combo_->blockSignals(true);
    voice_combo_->clear();
    voice_combo_->addItem("Auto (female preference)", "");

    const auto voices = voice_system_.list_installed_voices();
    for (const auto& v : voices) {
        QString label = QString::fromStdString(v.name);
        if (!v.gender.empty())
            label += "  [" + QString::fromStdString(v.gender) + "]";
        voice_combo_->addItem(label, QString::fromStdString(v.name));
    }
    voice_combo_->blockSignals(false);
}

// ── update_profile_display ───────────────────────────────────────────────────
void VoicePanel::update_profile_display()
{
    const VoiceProfile& p = voice_system_.get_voice_profile();
    name_label_->setText(
        QString::fromStdString(p.display_name) +
        "  ·  " + QString::fromStdString(p.language) +
        "  ·  pitch " + QString::number(static_cast<int>(p.pitch_hz)) + " Hz" +
        "  ·  rate " + QString::number(p.speaking_rate, 'f', 2) + "×");
}

void VoicePanel::set_status(const QString& msg, bool error)
{
    status_label_->setText(msg);
    status_label_->setStyleSheet(
        QString("color: %1; font-family: Consolas; font-size: 9px;"
                "letter-spacing: 2px; background: transparent;")
            .arg(error ? "#ff6060" : "#00ffc8"));
}

// ── Personality changed ───────────────────────────────────────────────────────
void VoicePanel::on_personality_changed(int index)
{
    const VoicePersonality personalities[] = {
        VoicePersonality::ARIA,
        VoicePersonality::NOVA,
        VoicePersonality::SAGE,
        VoicePersonality::ECHO
    };
    if (index < 0 || index > 3) return;

    voice_system_.apply_personality(personalities[index]);

    // Sync sliders to preset without re-triggering signals
    const VoiceProfile& p = voice_system_.get_voice_profile();
    pitch_slider_->blockSignals(true);
    rate_slider_->blockSignals(true);
    volume_slider_->blockSignals(true);
    pitch_slider_->setValue(static_cast<int>(p.pitch_hz));
    rate_slider_->setValue(static_cast<int>(p.speaking_rate * 100.f));
    volume_slider_->setValue(static_cast<int>(p.volume));
    pitch_slider_->blockSignals(false);
    rate_slider_->blockSignals(false);
    volume_slider_->blockSignals(false);

    pitch_val_label_->setText(QString::number(static_cast<int>(p.pitch_hz)) + " Hz");
    rate_val_label_->setText(QString::number(p.speaking_rate, 'f', 2) + "×");
    vol_val_label_->setText(QString::number(static_cast<int>(p.volume)) + "%");

    update_profile_display();
}

// ── SAPI voice override ───────────────────────────────────────────────────────
void VoicePanel::on_voice_changed(int index)
{
    const QString voice_name = voice_combo_->itemData(index).toString();
    VoiceProfile p = voice_system_.get_voice_profile();
    p.sapi_voice_name = voice_name.toStdString();
    voice_system_.set_voice_profile(p);
}

// ── Sliders ───────────────────────────────────────────────────────────────────
void VoicePanel::on_pitch_changed(int value)
{
    VoiceProfile p = voice_system_.get_voice_profile();
    p.pitch_hz = static_cast<float>(value);
    p.personality = VoicePersonality::Custom;
    voice_system_.set_voice_profile(p);
    pitch_val_label_->setText(QString::number(value) + " Hz");
    update_profile_display();
}

void VoicePanel::on_rate_changed(int value)
{
    VoiceProfile p = voice_system_.get_voice_profile();
    p.speaking_rate = value / 100.f;
    p.personality = VoicePersonality::Custom;
    voice_system_.set_voice_profile(p);
    rate_val_label_->setText(QString::number(p.speaking_rate, 'f', 2) + "×");
    update_profile_display();
}

void VoicePanel::on_volume_changed(int value)
{
    VoiceProfile p = voice_system_.get_voice_profile();
    p.volume = static_cast<float>(value);
    voice_system_.set_voice_profile(p);
    vol_val_label_->setText(QString::number(value) + "%");
}

// ── Speak button ──────────────────────────────────────────────────────────────
void VoicePanel::on_speak_button_clicked()
{
    if (pending_text_.isEmpty()) {
        set_status("● NO TEXT", true);
        return;
    }
    speak(pending_text_.toStdString());
}

// ── Test button ───────────────────────────────────────────────────────────────
void VoicePanel::on_test_button_clicked()
{
    const VoiceProfile& p = voice_system_.get_voice_profile();
    const std::string test_phrase =
        "BrainLLM neural interface online. "
        "I am " + p.profile_id + ", your AI voice assistant. "
        "All systems are operational.";
    speak(test_phrase);
}

// ── Speak public API ──────────────────────────────────────────────────────────
void VoicePanel::speak(const std::string& text)
{
    if (text.empty()) return;
    pending_text_ = QString::fromStdString(text);

    set_status("● SPEAKING");
    waveform_->set_active(true);

    voice_system_.speak_response(text);

    waveform_->set_active(false);
    set_status("● READY");
}

void VoicePanel::speak_greeting()
{
    set_status("● SPEAKING");
    waveform_->set_active(true);
    voice_system_.speak_startup_greeting();
    waveform_->set_active(false);
    set_status("● READY");
}

// ── Auto-speak toggle ─────────────────────────────────────────────────────────
void VoicePanel::on_speak_responses_toggled(bool checked)
{
    auto_speak_responses_ = checked;
    auto_speak_btn_->setText(checked ? "⬡ AUTO ON" : "⬡ AUTO");
    set_status(checked ? "● AUTO SPEAK ON" : "● READY");
}

// ── Mic / speech recognition ──────────────────────────────────────────────────
void VoicePanel::on_mic_button_clicked()
{
    if (rec_timer_->isActive()) {
        // Already listening — cancel
        rec_timer_->stop();
        waveform_->set_active(false);
        mic_button_->setText("◉ MIC");
        set_status("● READY");
        return;
    }

    if (!voice_system_.is_platform_speech_available()) {
        set_status("● NOT AVAILABLE", true);
        return;
    }

    mic_button_->setText("◉ STOP");
    set_status("● LISTENING");
    waveform_->set_active(true);
    waveform_->set_amplitude(0.9f);

    // Run recognition in a delayed call (blocking call via PS script)
    // We use a single-shot timer to let the UI refresh before blocking
    rec_timer_->setSingleShot(true);
    rec_timer_->start(50);
}

void VoicePanel::recognition_finished()
{
    const auto result = voice_system_.recognize_once(6);

    waveform_->set_active(false);
    mic_button_->setText("◉ MIC");

    if (result.success && !result.transcript.empty()) {
        const QString transcript = QString::fromStdString(result.transcript);
        const QString command = command_from_wake_word(transcript);
        if (command.isEmpty()) {
            set_status("● WAITING FOR WAKE WORD", true);
            pending_text_ = transcript;
            return;
        }

        set_status("● RECOGNIZED");
        pending_text_ = command;
        emit voice_input_ready(command);
    } else {
        set_status("● NOT RECOGNIZED", true);
    }
}

QString VoicePanel::command_from_wake_word(const QString& transcript)
{
    const WakeWordMatch match = detect_wake_word(transcript.toStdString(), wake_words_);
    if (!require_wake_word_) {
        return match.activated && !match.command.empty()
            ? QString::fromStdString(match.command).trimmed()
            : transcript.trimmed();
    }

    if (!match.activated) {
        return QString();
    }

    const QString command = QString::fromStdString(match.command).trimmed();
    return command.isEmpty() ? QString("How can I help?") : command;
}

} // namespace BrainLLM
