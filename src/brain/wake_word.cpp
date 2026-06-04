#include "wake_word.h"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace BrainLLM {

namespace {

std::string trim(const std::string& value) {
    size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start]))) {
        ++start;
    }

    size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
        --end;
    }

    return value.substr(start, end - start);
}

std::string normalize_for_match(const std::string& value) {
    std::string normalized;
    bool previous_space = false;
    for (unsigned char c : value) {
        if (std::isalnum(c)) {
            normalized += static_cast<char>(std::tolower(c));
            previous_space = false;
        } else if (!previous_space) {
            normalized += ' ';
            previous_space = true;
        }
    }
    return trim(normalized);
}

std::string strip_command_prefix(const std::string& original, const std::string& wake_word) {
    const std::string normalized_wake = normalize_for_match(wake_word);
    size_t word_count = 0;
    std::istringstream words(normalized_wake);
    std::string unused;
    while (words >> unused) {
        ++word_count;
    }

    size_t pos = 0;
    size_t consumed_words = 0;
    while (pos < original.size() && consumed_words < word_count) {
        while (pos < original.size() &&
               !std::isalnum(static_cast<unsigned char>(original[pos]))) {
            ++pos;
        }
        while (pos < original.size() &&
               std::isalnum(static_cast<unsigned char>(original[pos]))) {
            ++pos;
        }
        ++consumed_words;
    }

    while (pos < original.size()) {
        const unsigned char c = static_cast<unsigned char>(original[pos]);
        if (std::isalnum(c)) {
            break;
        }
        ++pos;
    }

    return trim(original.substr(pos));
}

} // namespace

std::vector<std::string> default_wake_words() {
    return {
        "hey brainllm",
        "brainllm",
        "hey siri",
        "siri",
        "alexa",
        "hey alexa",
        "ok brain"
    };
}

std::vector<std::string> parse_wake_words(const std::string& csv) {
    std::vector<std::string> words;
    std::istringstream stream(csv);
    std::string item;
    while (std::getline(stream, item, ',')) {
        item = trim(item);
        if (!item.empty()) {
            words.push_back(item);
        }
    }
    return words.empty() ? default_wake_words() : words;
}

WakeWordMatch detect_wake_word(const std::string& text, const std::vector<std::string>& wake_words) {
    WakeWordMatch match;
    const std::string normalized_text = normalize_for_match(text);
    if (normalized_text.empty()) {
        return match;
    }

    std::vector<std::string> sorted_words = wake_words.empty() ? default_wake_words() : wake_words;
    std::sort(sorted_words.begin(), sorted_words.end(), [](const std::string& a, const std::string& b) {
        return normalize_for_match(a).size() > normalize_for_match(b).size();
    });

    for (const std::string& wake_word : sorted_words) {
        const std::string normalized_wake = normalize_for_match(wake_word);
        if (normalized_wake.empty()) {
            continue;
        }

        const bool exact = normalized_text == normalized_wake;
        const bool prefix = normalized_text.rfind(normalized_wake + " ", 0) == 0;
        if (exact || prefix) {
            match.activated = true;
            match.wake_word = wake_word;
            match.command = strip_command_prefix(text, wake_word);
            return match;
        }
    }

    return match;
}

} // namespace BrainLLM
