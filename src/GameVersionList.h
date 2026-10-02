#ifndef MODINJECTOR2_GAMEVERSIONLIST_H
#define MODINJECTOR2_GAMEVERSIONLIST_H

#include <array>
#include <charconv>
#include <string_view>

namespace game_versions {
inline constexpr std::array<std::string_view, 6> supported = {
    "1.20.1",
    "1.20.6",
    "1.21.1",
    "1.21.8",
    "1.21.10"
};

inline bool parse(std::string_view version, std::array<int, 3>& parts) {
    size_t start = 0;
    for (size_t index = 0; index < parts.size(); ++index) {
        const size_t separator = version.find('.', start);
        const size_t end = separator == std::string_view::npos ? version.size() : separator;
        if (end == start || (index + 1 == parts.size() && separator != std::string_view::npos)) {
            return false;
        }
        const auto parsed = std::from_chars(version.data() + start, version.data() + end, parts[index]);
        if (parsed.ec != std::errc{} || parsed.ptr != version.data() + end) {
            return false;
        }
        if (index + 1 < parts.size() && separator == std::string_view::npos) {
            return false;
        }
        start = end + 1;
    }
    return true;
}

inline bool isValid(std::string_view version) {
    std::array<int, 3> parts{};
    return parse(version, parts);
}

inline bool usesNeoForge(std::string_view version) {
    std::array<int, 3> parts{};
    if (!parse(version, parts)) {
        return false;
    }
    return parts >= std::array<int, 3>{1, 21, 8};
}
}

#endif