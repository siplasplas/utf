#pragma once
// Deklaracje tablic danych Unicode
// Implementacja w generated/*.cpp

#include <cstdint>
#include <cstddef>
#include <utility>

namespace utf::data {

// ===== Case mapping data =====

// Simple toLower mapping: uppercase -> lowercase
extern const std::pair<char32_t, char32_t> lower_map[];
extern const size_t lower_map_size;

// Simple toUpper mapping: lowercase -> uppercase
extern const std::pair<char32_t, char32_t> upper_map[];
extern const size_t upper_map_size;

// Special case mapping (1:N)
struct SpecialCase {
    char32_t from;
    char32_t to[3];
    uint8_t len;
};

extern const SpecialCase special_upper[];
extern const size_t special_upper_size;

extern const SpecialCase special_lower[];
extern const size_t special_lower_size;

// ===== Decomposition data =====

struct Decomposition {
    char32_t from;
    char32_t base;      // base character
    char32_t combining; // combining mark (0 if none)
};

extern const Decomposition decomp_map[];
extern const size_t decomp_map_size;

// Aggressive folding for search
extern const std::pair<char32_t, char32_t> aggressive_fold[];
extern const size_t aggressive_fold_size;

struct AggressiveExpand {
    char32_t from;
    const char* to;
};

extern const AggressiveExpand aggressive_expand[];
extern const size_t aggressive_expand_size;

} // namespace utf::data
