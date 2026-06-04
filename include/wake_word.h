#pragma once

#include <string>
#include <vector>

namespace BrainLLM {

struct WakeWordMatch {
    bool activated = false;
    std::string wake_word;
    std::string command;
};

std::vector<std::string> default_wake_words();
std::vector<std::string> parse_wake_words(const std::string& csv);
WakeWordMatch detect_wake_word(const std::string& text,
                               const std::vector<std::string>& wake_words = default_wake_words());

} // namespace BrainLLM
