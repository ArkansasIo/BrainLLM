#pragma once

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QTimer>
#include <QVector>
#include <memory>
#include <vector>
#include "voice_audio_system.h"
#include "llm_engine.h"

namespace BrainLLM {

// ═══════════════════════════════════════════════════════════════════════════
// VoiceWaveform  — animated waveform bar display (HUD-style)
// ═══════════════════════════════════════════════════════════════════════════
class VoiceWaveform : public QWidget {
    Q_OBJECT
public:
    explicit VoiceWaveform(QWidget* parent = nullptr);
    void set_active(bool active);
    void set_amplitude(float amp); // 0..1

protected:
    void paintEvent(QPaintEvent*) override;

private slots:
    void tick();

private:
    QTimer*        anim_timer_;
    QVector<float> bars_;
    bool           active_;
    float          target_amp_;
    float          phase_;
};

// ═══════════════════════════════════════════════════════════════════════════
// VoicePanel  — full JARVIS-style AI voice control panel
// ═══════════════════════════════════════════════════════════════════════════
class VoicePanel : public QWidget {
    Q_OBJECT

public:
    explicit VoicePanel(QWidget* parent = nullptr);
    ~VoicePanel() = default;

    void set_llm_engine(std::shared_ptr<LLMEngine> engine);

    // Called by ChatWidget/MainWindow to speak a response
    void speak(const std::string& text);
    void speak_greeting();
    void set_wake_words(const std::vector<std::string>& wake_words);
    void set_wake_word_required(bool required);

signals:
    // Emitted when voice recognition produces a transcript
    void voice_input_ready(const QString& transcript);

private slots:
    void on_personality_changed(int index);
    void on_voice_changed(int index);
    void on_speak_button_clicked();
    void on_mic_button_clicked();
    void on_test_button_clicked();
    void on_pitch_changed(int value);
    void on_rate_changed(int value);
    void on_volume_changed(int value);
    void on_speak_responses_toggled(bool checked);
    void recognition_finished();

private:
    void build_ui();
    void refresh_voice_list();
    void update_profile_display();
    void set_status(const QString& msg, bool error = false);
    QString command_from_wake_word(const QString& transcript);

    std::shared_ptr<LLMEngine> llm_engine_;
    VoiceAudioSystem           voice_system_;
    bool                       auto_speak_responses_ = false;
    bool                       require_wake_word_ = true;
    std::vector<std::string>   wake_words_;

    // ── UI controls ──────────────────────────────────────────────────────────
    VoiceWaveform* waveform_;
    QLabel*        name_label_;
    QLabel*        status_label_;
    QComboBox*     personality_combo_;
    QComboBox*     voice_combo_;
    QSlider*       pitch_slider_;
    QSlider*       rate_slider_;
    QSlider*       volume_slider_;
    QLabel*        pitch_val_label_;
    QLabel*        rate_val_label_;
    QLabel*        vol_val_label_;
    QPushButton*   speak_button_;
    QPushButton*   mic_button_;
    QPushButton*   test_button_;
    QPushButton*   auto_speak_btn_;

    // Pending text to speak (for async-style dispatch)
    QString        pending_text_;

    // Recognition async timer
    QTimer*        rec_timer_;
};

} // namespace BrainLLM
