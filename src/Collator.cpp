// Collator implementation

#include "utf/UTF.hpp"
#include "utf/Collator.hpp"
#include <cstring>
#include <algorithm>

namespace utf {

Collator::Collator(const char* locale) {
    // Find locale data
    for (size_t i = 0; i < data::locale_collations_size; i++) {
        if (std::strcmp(data::locale_collations[i].locale, locale) == 0) {
            m_data = &data::locale_collations[i];
            m_locale = data::locale_collations[i].locale;
            return;
        }
    }

    // Fallback to root
    for (size_t i = 0; i < data::locale_collations_size; i++) {
        if (std::strcmp(data::locale_collations[i].locale, "root") == 0) {
            m_data = &data::locale_collations[i];
            m_locale = "root";
            return;
        }
    }
}

void Collator::getWeight(char32_t cp, uint16_t& primary, uint8_t& secondary, uint8_t& tertiary) const {
    if (!m_data) {
        // No data, use code point as primary weight
        primary = static_cast<uint16_t>(cp > 0xFFFF ? 0xFFFF : cp);
        secondary = 0;
        tertiary = 0;
        return;
    }

    // Binary search in single-char table
    size_t lo = 0, hi = m_data->single_size;
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        if (m_data->single[mid].cp == cp) {
            primary = m_data->single[mid].primary;
            secondary = m_data->single[mid].secondary;
            tertiary = m_data->single[mid].tertiary;
            return;
        }
        if (m_data->single[mid].cp < cp)
            lo = mid + 1;
        else
            hi = mid;
    }

    // Not found - use code point as weight
    // Give non-Latin characters higher primary weights
    if (cp < 128) {
        // ASCII: use code point directly, scaled
        primary = static_cast<uint16_t>(cp * 10);
    } else {
        // Non-ASCII: place after Z (350+)
        primary = static_cast<uint16_t>(350 + (cp % 0x1000));
    }
    secondary = 0;
    tertiary = 0;
}

size_t Collator::matchContraction(const char* str, size_t remaining,
                                   uint16_t& primary, uint8_t& secondary, uint8_t& tertiary) const {
    if (!m_data || !m_data->multi || m_data->multi_size == 0)
        return 0;

    // Check each contraction
    for (size_t i = 0; i < m_data->multi_size; i++) {
        const char* pattern = m_data->multi[i].chars;
        size_t plen = std::strlen(pattern);
        if (plen <= remaining && std::strncmp(str, pattern, plen) == 0) {
            primary = m_data->multi[i].primary;
            secondary = m_data->multi[i].secondary;
            tertiary = m_data->multi[i].tertiary;
            return plen;
        }
    }
    return 0;
}

int Collator::compare(const std::string_view& a, const std::string_view& b) const {
    ::UTF utf;

    const char* pa = a.data();
    const char* ea = a.data() + a.size();
    const char* pb = b.data();
    const char* eb = b.data() + b.size();

    // Collect weights for comparison
    std::vector<uint32_t> wa, wb;

    // Process string a
    while (pa < ea) {
        uint16_t primary;
        uint8_t secondary, tertiary;

        // Check for contraction
        size_t clen = matchContraction(pa, ea - pa, primary, secondary, tertiary);
        if (clen > 0) {
            pa += clen;
        } else {
            // Single character
            const char* next;
            char32_t cp = utf.codePointAt(pa, ea, &next);
            getWeight(cp, primary, secondary, tertiary);
            pa = next;
        }

        // Pack weight based on strength
        uint32_t weight = primary << 16;
        if (m_strength >= CollationStrength::Secondary)
            weight |= secondary << 8;
        if (m_strength >= CollationStrength::Tertiary)
            weight |= tertiary;
        wa.push_back(weight);
    }

    // Process string b
    while (pb < eb) {
        uint16_t primary;
        uint8_t secondary, tertiary;

        size_t clen = matchContraction(pb, eb - pb, primary, secondary, tertiary);
        if (clen > 0) {
            pb += clen;
        } else {
            const char* next;
            char32_t cp = utf.codePointAt(pb, eb, &next);
            getWeight(cp, primary, secondary, tertiary);
            pb = next;
        }

        uint32_t weight = primary << 16;
        if (m_strength >= CollationStrength::Secondary)
            weight |= secondary << 8;
        if (m_strength >= CollationStrength::Tertiary)
            weight |= tertiary;
        wb.push_back(weight);
    }

    // Compare weight arrays
    size_t minLen = std::min(wa.size(), wb.size());
    for (size_t i = 0; i < minLen; i++) {
        if (wa[i] < wb[i]) return -1;
        if (wa[i] > wb[i]) return 1;
    }

    // If all compared weights are equal, shorter string comes first
    if (wa.size() < wb.size()) return -1;
    if (wa.size() > wb.size()) return 1;
    return 0;
}

int Collator::compare(const std::u32string_view& a, const std::u32string_view& b) const {
    // For u32 strings, simpler processing (no contractions for now)
    std::vector<uint32_t> wa, wb;

    for (char32_t cp : a) {
        uint16_t primary;
        uint8_t secondary, tertiary;
        getWeight(cp, primary, secondary, tertiary);

        uint32_t weight = primary << 16;
        if (m_strength >= CollationStrength::Secondary)
            weight |= secondary << 8;
        if (m_strength >= CollationStrength::Tertiary)
            weight |= tertiary;
        wa.push_back(weight);
    }

    for (char32_t cp : b) {
        uint16_t primary;
        uint8_t secondary, tertiary;
        getWeight(cp, primary, secondary, tertiary);

        uint32_t weight = primary << 16;
        if (m_strength >= CollationStrength::Secondary)
            weight |= secondary << 8;
        if (m_strength >= CollationStrength::Tertiary)
            weight |= tertiary;
        wb.push_back(weight);
    }

    size_t minLen = std::min(wa.size(), wb.size());
    for (size_t i = 0; i < minLen; i++) {
        if (wa[i] < wb[i]) return -1;
        if (wa[i] > wb[i]) return 1;
    }

    if (wa.size() < wb.size()) return -1;
    if (wa.size() > wb.size()) return 1;
    return 0;
}

std::vector<uint8_t> Collator::getSortKey(const std::string_view& str) const {
    ::UTF utf;
    std::vector<uint8_t> key;

    const char* p = str.data();
    const char* e = str.data() + str.size();

    // Collect primary weights
    std::vector<uint16_t> primaries;
    std::vector<uint8_t> secondaries;
    std::vector<uint8_t> tertiaries;

    while (p < e) {
        uint16_t primary;
        uint8_t secondary, tertiary;

        size_t clen = matchContraction(p, e - p, primary, secondary, tertiary);
        if (clen > 0) {
            p += clen;
        } else {
            const char* next;
            char32_t cp = utf.codePointAt(p, e, &next);
            getWeight(cp, primary, secondary, tertiary);
            p = next;
        }

        primaries.push_back(primary);
        secondaries.push_back(secondary);
        tertiaries.push_back(tertiary);
    }

    // Build sort key: primaries | separator | secondaries | separator | tertiaries
    for (uint16_t p : primaries) {
        key.push_back(static_cast<uint8_t>(p >> 8));
        key.push_back(static_cast<uint8_t>(p & 0xFF));
    }

    if (m_strength >= CollationStrength::Secondary) {
        key.push_back(0);  // Separator
        for (uint8_t s : secondaries) {
            key.push_back(s);
        }
    }

    if (m_strength >= CollationStrength::Tertiary) {
        key.push_back(0);  // Separator
        for (uint8_t t : tertiaries) {
            key.push_back(t);
        }
    }

    return key;
}

std::vector<uint8_t> Collator::getSortKey(const std::u32string_view& str) const {
    std::vector<uint8_t> key;

    std::vector<uint16_t> primaries;
    std::vector<uint8_t> secondaries;
    std::vector<uint8_t> tertiaries;

    for (char32_t cp : str) {
        uint16_t primary;
        uint8_t secondary, tertiary;
        getWeight(cp, primary, secondary, tertiary);

        primaries.push_back(primary);
        secondaries.push_back(secondary);
        tertiaries.push_back(tertiary);
    }

    for (uint16_t p : primaries) {
        key.push_back(static_cast<uint8_t>(p >> 8));
        key.push_back(static_cast<uint8_t>(p & 0xFF));
    }

    if (m_strength >= CollationStrength::Secondary) {
        key.push_back(0);
        for (uint8_t s : secondaries) {
            key.push_back(s);
        }
    }

    if (m_strength >= CollationStrength::Tertiary) {
        key.push_back(0);
        for (uint8_t t : tertiaries) {
            key.push_back(t);
        }
    }

    return key;
}

} // namespace utf
