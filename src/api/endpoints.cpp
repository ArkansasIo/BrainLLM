#include "rest_server.h"
#include <QStringList>

namespace BrainLLM {

QStringList documented_endpoint_paths() {
    return {
        "/",
        "/client",
        "/api/process",
        "/api/generate",
        "/api/chat",
        "/api/status",
        "/api/health",
        "/api/memory",
        "/api/config",
        "/api/endpoints",
        "/api/algorithms",
        "/api/train",
        "/api/reset",
        "/api/airllm/config",
        "/api/wolfram",
        "/api/speech/status",
        "/api/speech/speak",
        "/api/speech/synthesize",
        "/api/speech/recognize",
        "/api/scripts",
        "/api/scripts/execute",
        "/v1/models",
        "/v1/completions",
        "/v1/chat/completions"
    };
}

} // namespace BrainLLM
