#pragma once

#include <QString>

namespace BrainLLM {

struct WolframAlphaResult {
    bool success;
    QString answer;
    QString error;
    QString source_url;
};

class WolframAlphaClient {
public:
    WolframAlphaClient();

    bool is_configured() const;
    QString app_id() const;

    WolframAlphaResult short_answer(const QString& query, int timeout_ms = 8000) const;
    WolframAlphaResult llm_answer(const QString& query, int timeout_ms = 8000) const;

private:
    QString app_id_;

    WolframAlphaResult request_text_result(
        const QString& endpoint,
        const QString& query,
        const QString& input_parameter,
        int timeout_ms
    ) const;
};

} // namespace BrainLLM
