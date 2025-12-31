#pragma once
// Collator - locale-aware string comparison
// Based on CLDR collation data

#include <cstdint>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace utf {

// Forward declaration
struct UTF;

namespace data {

// Single character collation weight
struct CollationWeight {
    char32_t cp;
    uint16_t primary;
    uint8_t secondary;
    uint8_t tertiary;
};

// Multi-character sequence (contraction), e.g., "ch" in Czech
struct CollationContraction {
    const char* chars;  // UTF-8 string
    uint16_t primary;
    uint8_t secondary;
    uint8_t tertiary;
};

// Locale collation data
struct LocaleCollation {
    const char* locale;
    const CollationWeight* single;
    size_t single_size;
    const CollationContraction* multi;
    size_t multi_size;
};

// Extern declarations for generated data
extern const LocaleCollation locale_collations[];
extern const size_t locale_collations_size;

} // namespace data

// Collation strength levels
enum class CollationStrength {
    Primary,    // Ignore accents and case (a = á = A)
    Secondary,  // Consider accents, ignore case (a = A < á)
    Tertiary    // Consider all differences (a < A < á)
};

class Collator {
public:
    // Create collator for specified locale (e.g., "pl", "cs", "de")
    // Falls back to "root" if locale not found
    explicit Collator(const char* locale = "root");
    explicit Collator(const std::string& locale) : Collator(locale.c_str()) {}

    // Compare two strings
    // Returns: <0 if a<b, 0 if a==b, >0 if a>b
    int compare(const std::string_view& a, const std::string_view& b) const;
    int compare(const std::u32string_view& a, const std::u32string_view& b) const;

    // Generate sort key for efficient sorting of many strings
    std::vector<uint8_t> getSortKey(const std::string_view& str) const;
    std::vector<uint8_t> getSortKey(const std::u32string_view& str) const;

    // Settings
    void setStrength(CollationStrength strength) { m_strength = strength; }
    CollationStrength getStrength() const { return m_strength; }

    // Get locale name
    const char* getLocale() const { return m_locale; }

private:
    // Get collation weight for a code point
    void getWeight(char32_t cp, uint16_t& primary, uint8_t& secondary, uint8_t& tertiary) const;

    // Check for contraction match at current position
    // Returns length of contraction in bytes (0 if no match)
    size_t matchContraction(const char* str, size_t remaining,
                            uint16_t& primary, uint8_t& secondary, uint8_t& tertiary) const;

    const data::LocaleCollation* m_data = nullptr;
    const char* m_locale = "root";
    CollationStrength m_strength = CollationStrength::Tertiary;
};

} // namespace utf
