//
// Created by andrzej on 8/27/22.
//
#include <gtest/gtest.h>
#include "utf/UTF.hpp"

bool skipHard = false;

using namespace std;

u32string fillDstring() {
    const int MAX = UTF::MaxCP;
    u32string dstr;
    dstr.resize(MAX + 1);
    dstr[0] = 1;
    for (int i = 1; i <= MAX; i++)
        if (UTF::isSurrogate(i))
            dstr[i] = 1;
        else
            dstr[i] = i;
    return dstr;
}

TEST(Conv, fromUTF32) {
    if (skipHard)
        GTEST_SKIP();
    u32string dstr = fillDstring();
    UTF utf;
    string str = utf.fromUTF32(dstr);
    u32string dstr1 = utf.toUTF32(str);
    bool fail32to8 = false;
    EXPECT_EQ(dstr.size(), dstr1.size());
    for (int i = 0; i <= dstr.size(); i++) {
        if (dstr[i] != dstr1[i]) {
            fail32to8 = true;
            break;
        }
    }
    EXPECT_FALSE(fail32to8);
}

TEST(Conv, fromUTF32to16) {
    if (skipHard)
        GTEST_SKIP();
    u32string dstr = fillDstring();
    UTF utf;
    u16string wstr = utf.fromUTF32to16(dstr);
    u32string dstr1 = utf.toUTF32(wstr);
    bool fail32to16 = false;
    EXPECT_EQ(dstr.size(), dstr1.size());
    for (int i = 0; i <= dstr.size(); i++) {
        if (dstr[i] != dstr1[i]) {
            fail32to16 = true;
            break;
        }
    }
    EXPECT_FALSE(fail32to16);
}

TEST(Conv, toUTF16) {
    if (skipHard)
        GTEST_SKIP();
    u32string dstr = fillDstring();
    UTF utf;
    string str = utf.fromUTF32(dstr);
    u16string wstr = utf.toUTF16(str);
    string str1 = utf.toUTF8(wstr);
    bool fail8to16 = false;
    EXPECT_EQ(str.size(), str1.size());
    for (int i = 0; i <= str.size(); i++) {
        if (str[i] != str1[i]) {
            fail8to16 = true;
            break;
        }
    }
    EXPECT_FALSE(fail8to16);
}

TEST(Conv, toUTF8) {
    if (skipHard)
        GTEST_SKIP();
    u32string dstr = fillDstring();
    UTF utf;
    u16string wstr = utf.fromUTF32to16(dstr);
    string str = utf.toUTF8(wstr);
    u16string wstr1 = utf.toUTF16(str);
    bool fail16to8 = false;
    EXPECT_EQ(wstr.size(), wstr1.size());
    for (int i = 0; i <= wstr.size(); i++) {
        if (wstr[i] != wstr1[i]) {
            fail16to8 = true;
            break;
        }
    }
    EXPECT_FALSE(fail16to8);
}

TEST(Conv, toUTF8Z) {
    if (skipHard)
        GTEST_SKIP();
    u32string dstr = fillDstring();
    UTF utf;
    u16string wstr = utf.fromUTF32to16(dstr);
    string str = utf.toUTF8(wstr.c_str());
    u16string wstr1 = utf.toUTF16(str.c_str());
    bool fail16to8 = false;
    EXPECT_EQ(wstr.size(), wstr1.size());
    for (int i = 0; i <= wstr.size(); i++) {
        if (wstr[i] != wstr1[i]) {
            fail16to8 = true;
            break;
        }
    }
    EXPECT_FALSE(fail16to8);
}

TEST(Errors, on1) {
    UTF utf;
    string str = "b\xc4\x85k";
    u16string wstr = utf.toUTF16(str);
    EXPECT_EQ(wstr, u"b\u0105k");
    string str1 = "b\xc4k";
    u16string wstr1 = utf.toUTF16(str1);
    EXPECT_EQ(wstr1, u"b\ufffdk");
}

TEST(CorrectUtf8, len1) {
    string str = "a\106b";
    u32string expect{'a', 0106, 'b'};
    UTF utf;
    u32string dstr = utf.toUTF32(str);
    u16string wstr = utf.fromUTF32to16(dstr);
    EXPECT_EQ(dstr, expect);
    EXPECT_EQ(wstr, u"a\106b");
}

//110xxxxx 10xxxxxx
TEST(CorrectUtf8, len2) {
    string str = "a\325\252b";
    u32string expect{'a', 02552, 'b'};
    UTF utf;
    u32string dstr = utf.toUTF32(str);
    u16string wstr = utf.fromUTF32to16(dstr);
    EXPECT_EQ(dstr, expect);
    EXPECT_EQ(wstr, u"a\x056a\x0062");
}

//1110xxxx 10xxxxxx 10xxxxxx
TEST(CorrectUtf8, len3) {
    string str = "a\352\252\252b";
    u32string expect{'a', 0xaaaa, 'b'};
    UTF utf;
    u32string dstr = utf.toUTF32(str);
    u16string wstr = utf.fromUTF32to16(dstr);
    EXPECT_EQ(dstr, expect);
    EXPECT_EQ(wstr, u"a\uaaaab");
}

//http://russellcottrell.com/greek/utilities/SurrogatePairCalculator.htm
//11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

TEST(AmbigUtf8, slash2) {
    string str = "a\300\257b";
    //11000000 10101111
    u32string expect{'a', 0xfffd, 'b'};
    UTF utf;
    u32string dstr = utf.toUTF32(str);
    EXPECT_EQ(utf.errambig, 1);
    u16string wstr = utf.fromUTF32to16(dstr);
    EXPECT_EQ(dstr, expect);
    EXPECT_EQ(wstr, u"a\xfffd\x0062");
}

TEST(AmbigUtf8, slash3) {
    string str = "a\340\200\257b";
    //11100000 10000000 10101111
    u32string expect{'a', 0xfffd, 'b'};
    UTF utf;
    u32string dstr = utf.toUTF32(str);
    EXPECT_EQ(utf.errambig, 1);
    u16string wstr = utf.fromUTF32to16(dstr);
    EXPECT_EQ(dstr, expect);
    EXPECT_EQ(wstr, u"a\xfffd\x0062");
}

TEST(AmbigUtf8, len4) {
    string str = "a\360\200\200\203b";
    //11110000 10000000 10000000 10000011
    u32string expect{'a', 0xfffd, 'b'};
    UTF utf;
    u32string dstr = utf.toUTF32(str);
    EXPECT_EQ(utf.errambig, 1);
    u16string wstr = utf.fromUTF32to16(dstr);
    EXPECT_EQ(dstr, expect);
    EXPECT_EQ(wstr, u"a\xfffd\x0062");
}

TEST(ExceedsUtf16, len4) {
    string str = "a\367\277\277\277b";
    //11110111 10111111 10111111 10111111
    u32string expect{'a', 0x1FFFFF, 'b'};
    UTF utf;
    u32string dstr = utf.toUTF32(str);
    EXPECT_EQ(utf.errors, 0);
    u16string wstr = utf.fromUTF32to16(dstr);
    EXPECT_EQ(utf.errors, 1);
    EXPECT_EQ(dstr, expect);
    EXPECT_EQ(wstr, u"a\xfffd\x0062");
}

TEST(ExceedsUtf16, len5) {
    string str = "a\372\200\200\200\200b";
    //11111010 10000000 10000000 10000000 10000000
    //10000000000000000000000000
    u32string expect{'a', 0x2000000, 'b'};
    UTF utf;
    u32string dstr = utf.toUTF32(str);
    EXPECT_EQ(utf.errors, 0);
    u16string wstr = utf.fromUTF32to16(dstr);
    EXPECT_EQ(utf.errors, 1);
    EXPECT_EQ(dstr, expect);
    EXPECT_EQ(wstr, u"a\xfffd\x0062");
}

TEST(ExceedsUtf16, len6) {
    string str = "a\375\200\200\200\200\200b";
    //1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
    //1000000000000000000000000000000
    u32string expect{'a', 0x40000000, 'b'};
    UTF utf;
    u32string dstr = utf.toUTF32(str);
    EXPECT_EQ(utf.errors, 0);
    u16string wstr = utf.fromUTF32to16(dstr);
    EXPECT_EQ(utf.errors, 1);
    EXPECT_EQ(dstr, expect);
    EXPECT_EQ(wstr, u"a\xfffd\x0062");
}

//form 10xxxxxx without start byte
TEST(ut8errors, inside) {
    for (int len = 1; len < 10; len++) {
        unsigned char c = 128 + len;
        string str = "a";
        u32string expect{'a'};
        for (int j = 0; j < len; j++) {
            str += (char) c;
            expect.push_back(0xfffd);
        }
        str += "b";
        expect.push_back('b');
        UTF utf;
        u32string dstr = utf.toUTF32(str);
        EXPECT_EQ(utf.errors, len);
        EXPECT_EQ(dstr, expect);
    }
}

//no form 10xxxxxx after head 110xxxxx
TEST(utf8errors, onlyHead) {
    string str = "a\337b";
    u32string expect{'a', 0xfffd, 'b'};
    UTF utf;
    u32string dstr = utf.toUTF32(str);
    EXPECT_EQ(utf.errors, 1);
    EXPECT_EQ(dstr, expect);
}

TEST(utf8errors, headAndLessBytes) {
    string str = "a\357\252b";
    u32string expect{'a', 0xfffd, 'b'};
    UTF utf;
    u32string dstr = utf.toUTF32(str);
    EXPECT_EQ(utf.errors, 1);
    EXPECT_EQ(dstr, expect);
}

TEST(utf8errors, twoHeads) {
    string str = "a\357\337b";
    u32string expect{'a', 0xfffd, 0xfffd, 'b'};
    UTF utf;
    u32string dstr = utf.toUTF32(str);
    EXPECT_EQ(utf.errors, 2);
    EXPECT_EQ(dstr, expect);
}

TEST(Find, len6) {
    string str = "a\375\200\201\202\203\204\205b";
    const char *s = str.c_str() + 1;
    UTF utf;
    for (int i = 0; i < 6; i++) {
        const char *bs = utf.findUtf8(s + i, s);
        EXPECT_EQ(bs, s);
    }
    for (int i = 6; i <= 7; i++) {
        const char *bs = utf.findUtf8(s + i, s);
        EXPECT_EQ(bs, s + i);
    }
}

TEST(Find, len4) {
    string str = "a\360\200\201\202\203\204\205b";
    const char *s = str.c_str() + 1;
    UTF utf;
    for (int i = 0; i < 4; i++) {
        const char *bs = utf.findUtf8(s + i, s);
        EXPECT_EQ(bs, s);
    }
    for (int i = 4; i <= 7; i++) {
        const char *bs = utf.findUtf8(s + i, s);
        EXPECT_EQ(bs, s + i);
    }
}

TEST(Len, simple) {
    string str = "bąk";
    UTF utf;
    EXPECT_EQ(utf.countCodePoints(str), 3);
    EXPECT_EQ(utf.length16(str), 3);
}

TEST(Ncodes, forback) {
    string str = "bąkαβγAδ";
    const char *s = str.c_str();
    const char *sstart = s;
    const char *send = s + str.length();
    EXPECT_EQ(*send, 0);
    EXPECT_EQ(*s, 'b');
    int64_t actual;
    UTF utf;
    s = utf.forwardNcodes(s, 2, send, actual);
    EXPECT_EQ(*s, 'k');
    EXPECT_EQ(actual, 2);
    const char *sinside = s + 2;
    s = utf.forwardNcodes(s, 4, send, actual);
    EXPECT_EQ(*s, 'A');
    EXPECT_EQ(actual, 4);
    s = utf.forwardNcodes(s, 3, send, actual);
    EXPECT_EQ(*s, 0);
    EXPECT_EQ(actual, 2);
    s = utf.forwardNcodes(s, 3, send, actual);
    EXPECT_EQ(*s, 0);
    EXPECT_EQ(actual, 0);
    s = utf.backwardNcodes(s, 2, sstart, actual);
    EXPECT_EQ(*s, 'A');
    EXPECT_EQ(actual, 2);
    s = utf.backwardNcodes(s, 4, sstart, actual);
    EXPECT_EQ(*s, 'k');
    EXPECT_EQ(actual, 4);
    s = utf.backwardNcodes(s, 3, sstart, actual);
    EXPECT_EQ(*s, 'b');
    EXPECT_EQ(actual, 2);
    s = utf.backwardNcodes(s, 3, sstart, actual);
    EXPECT_EQ(*s, 'b');
    EXPECT_EQ(actual, 0);
    s = utf.forwardNcodes(sinside, 3, send, actual);
    EXPECT_EQ(*s, 'A');
    EXPECT_EQ(actual, 3);
    s = utf.backwardNcodes(sinside, 2, sstart, actual);
    EXPECT_EQ(*s, 'k');
    EXPECT_EQ(actual, 2);
}

TEST(Substr, Unicode) {
    UTF utf;
    string str = "01.123ąęć1\U00013032А\U00013032БВГДЕαβεζηλ345";
    u16string wstr = utf.toUTF16(str);
    u32string dstr = utf.toUTF32(str);
    for (int i = -2; i <= (int) dstr.size() + 1; i++) {
        for (int j = i - 2; j < (int) dstr.size() + 1; j++) {
            u32string sub32to32 = utf.substr32(dstr, i, j - i);
            string sub32to32to8 = utf.fromUTF32(sub32to32);
            u16string sub32to32to16 = utf.fromUTF32to16(sub32to32);
            string sub8to8 = utf.substr8(str, i, j - i);
            EXPECT_EQ(sub8to8, sub32to32to8);
            string sub16to8 = utf.substr8From16(wstr, i, j - i);
            EXPECT_EQ(sub16to8, sub32to32to8);
            u16string sub8to16 = utf.substrToUTF16(str, i, j - i);
            EXPECT_EQ(sub8to16, sub32to32to16);
            u16string sub16to16 = utf.substr16(wstr, i, j - i);
            EXPECT_EQ(sub16to16, sub32to32to16);
            u32string sub8to32 = utf.substrToUTF32(str, i, j - i);
            EXPECT_EQ(sub8to32, sub32to32);
        }
    }
}

TEST(Subview, Unicode) {
    UTF utf;
    string str = "01.123ąęć1\U00013032А\U00013032БВГДЕαβεζηλ345";
    auto view8 = std::string_view(str);
    u16string wstr = utf.toUTF16(str);
    auto view16 = u16string_view(wstr);
    u32string dstr = utf.toUTF32(str);
    for (int i = -2; i <= (int) dstr.size() + 1; i++) {
        for (int j = i - 2; j < (int) dstr.size() + 1; j++) {
            u32string sub32to32 = utf.substr32(dstr, i, j - i);
            std::string sub8 = utf.fromUTF32(sub32to32);
            std::u16string sub16 = utf.fromUTF32to16(sub32to32);
            auto subview8 = std::string_view(sub8);
            auto subview16 = u16string_view(sub16);
            auto subview8a = utf.subview8(str, i, j - i);
            auto subview8b = utf.subview8(view8, i, j - i);
            auto subview16a = utf.subview16(wstr, i, j - i);
            auto subview16b = utf.subview16(view16, i, j - i);
            EXPECT_EQ(subview8a, subview8);
            EXPECT_EQ(subview8b, subview8);
            EXPECT_EQ(subview16a, subview16);
            EXPECT_EQ(subview16b, subview16);
        }
    }
}

TEST(Endianness, Swap16) {
    char16_t c = 0x1234;
    char16_t expected = 0x3412;
    char16_t test = UTF::swap16(c);
    EXPECT_EQ(test, expected);
}

TEST(Endianness, Swap16string) {
    char16_t c = 0x1234;
    std::u16string s16;
    s16 = c + c;
    char16_t expected = 0x3412;
    std::u16string s16exp;
    s16exp = expected + expected;
    UTF::swapIt(s16);
    EXPECT_EQ(s16, s16exp);
}

TEST(Endianness, Swap32) {
    char32_t c = 0x12345678;
    char32_t expected = 0x56781234;
    char32_t test = UTF::swap32(c);
    EXPECT_EQ(test, expected);
}

TEST(Endianness, Reverse32) {
    char32_t c = 0x12345678;
    char32_t expected = 0x78563412;
    char32_t test = UTF::reverse32(c);
    EXPECT_EQ(test, expected);
}

TEST(Endianness, Reverse32string) {
    char32_t c = 0x12345678;
    std::u32string s32;
    s32 = c + c;
    char32_t expected = 0x78563412;
    std::u32string s32exp;
    s32exp = expected + expected;
    UTF::reverseIt(s32);
    EXPECT_EQ(s32, s32exp);
}

// ===== Case mapping tests =====

TEST(CaseMapping, PolishLetters) {
    UTF utf;
    // Polish lowercase -> uppercase
    EXPECT_EQ(UTF::toUpperCodePoint(U'ą'), U'Ą');
    EXPECT_EQ(UTF::toUpperCodePoint(U'ć'), U'Ć');
    EXPECT_EQ(UTF::toUpperCodePoint(U'ę'), U'Ę');
    EXPECT_EQ(UTF::toUpperCodePoint(U'ł'), U'Ł');
    EXPECT_EQ(UTF::toUpperCodePoint(U'ń'), U'Ń');
    EXPECT_EQ(UTF::toUpperCodePoint(U'ó'), U'Ó');
    EXPECT_EQ(UTF::toUpperCodePoint(U'ś'), U'Ś');
    EXPECT_EQ(UTF::toUpperCodePoint(U'ź'), U'Ź');
    EXPECT_EQ(UTF::toUpperCodePoint(U'ż'), U'Ż');

    // Polish uppercase -> lowercase
    EXPECT_EQ(UTF::toLowerCodePoint(U'Ą'), U'ą');
    EXPECT_EQ(UTF::toLowerCodePoint(U'Ć'), U'ć');
    EXPECT_EQ(UTF::toLowerCodePoint(U'Ę'), U'ę');
    EXPECT_EQ(UTF::toLowerCodePoint(U'Ł'), U'ł');
    EXPECT_EQ(UTF::toLowerCodePoint(U'Ń'), U'ń');
    EXPECT_EQ(UTF::toLowerCodePoint(U'Ó'), U'ó');
    EXPECT_EQ(UTF::toLowerCodePoint(U'Ś'), U'ś');
    EXPECT_EQ(UTF::toLowerCodePoint(U'Ź'), U'ź');
    EXPECT_EQ(UTF::toLowerCodePoint(U'Ż'), U'ż');
}

TEST(CaseMapping, ASCIILetters) {
    for (char32_t c = U'a'; c <= U'z'; c++) {
        EXPECT_EQ(UTF::toUpperCodePoint(c), c - 32);
    }
    for (char32_t c = U'A'; c <= U'Z'; c++) {
        EXPECT_EQ(UTF::toLowerCodePoint(c), c + 32);
    }
}

TEST(CaseMapping, CyrillicLetters) {
    // Russian lowercase а-я (0x0430-0x044F) -> uppercase А-Я (0x0410-0x042F)
    for (char32_t c = 0x0430; c <= 0x044F; c++) {
        EXPECT_EQ(UTF::toUpperCodePoint(c), c - 32);
    }
    for (char32_t c = 0x0410; c <= 0x042F; c++) {
        EXPECT_EQ(UTF::toLowerCodePoint(c), c + 32);
    }
}

TEST(CaseMapping, GermanSS) {
    // ß -> SS (special 1:N mapping)
    UTF utf;
    u32string lower = U"straße";
    u32string upper = utf.toUpper(lower);
    EXPECT_EQ(upper, U"STRASSE");

    // SS -> ss (simple)
    u32string upperInput = U"STRASSE";
    u32string lowerResult = utf.toLower(upperInput);
    EXPECT_EQ(lowerResult, U"strasse");
}

TEST(CaseMapping, UTF8Convenience) {
    UTF utf;
    // Polish text
    std::string lower = "zażółć gęślą jaźń";
    std::string upper = utf.toUpper8(lower);
    EXPECT_EQ(upper, "ZAŻÓŁĆ GĘŚLĄ JAŹŃ");

    std::string back = utf.toLower8(upper);
    EXPECT_EQ(back, lower);
}

TEST(CaseMapping, NoChange) {
    // Numbers and symbols should not change
    EXPECT_EQ(UTF::toUpperCodePoint(U'0'), U'0');
    EXPECT_EQ(UTF::toLowerCodePoint(U'0'), U'0');
    EXPECT_EQ(UTF::toUpperCodePoint(U'@'), U'@');
    EXPECT_EQ(UTF::toLowerCodePoint(U'@'), U'@');
    EXPECT_EQ(UTF::toUpperCodePoint(U'中'), U'中'); // Chinese character
    EXPECT_EQ(UTF::toLowerCodePoint(U'中'), U'中');
}

TEST(CaseMapping, GreekLetters) {
    UTF utf;
    // Greek lowercase α-ω -> uppercase Α-Ω
    EXPECT_EQ(UTF::toUpperCodePoint(U'α'), U'Α');
    EXPECT_EQ(UTF::toUpperCodePoint(U'β'), U'Β');
    EXPECT_EQ(UTF::toUpperCodePoint(U'γ'), U'Γ');
    EXPECT_EQ(UTF::toUpperCodePoint(U'ω'), U'Ω');

    EXPECT_EQ(UTF::toLowerCodePoint(U'Α'), U'α');
    EXPECT_EQ(UTF::toLowerCodePoint(U'Β'), U'β');
    EXPECT_EQ(UTF::toLowerCodePoint(U'Γ'), U'γ');
    EXPECT_EQ(UTF::toLowerCodePoint(U'Ω'), U'ω');
}

TEST(CaseMapping, SpecialCases) {
    UTF utf;
    // fi ligature (ﬁ U+FB01) -> FI
    u32string fi = U"\uFB01";
    u32string FI = utf.toUpper(fi);
    EXPECT_EQ(FI, U"FI");

    // fl ligature (ﬂ U+FB02) -> FL
    u32string fl = U"\uFB02";
    u32string FL = utf.toUpper(fl);
    EXPECT_EQ(FL, U"FL");
}

// ===== Accent folding tests =====

TEST(AccentFolding, StandardPolish) {
    // Standard folding: ą→a, ę→e, ó→o, etc.
    // But ł stays ł (separate letter, not a+combining)
    EXPECT_EQ(UTF::foldAccent(U'ą'), U'a');
    EXPECT_EQ(UTF::foldAccent(U'Ą'), U'A');
    EXPECT_EQ(UTF::foldAccent(U'ć'), U'c');
    EXPECT_EQ(UTF::foldAccent(U'Ć'), U'C');
    EXPECT_EQ(UTF::foldAccent(U'ę'), U'e');
    EXPECT_EQ(UTF::foldAccent(U'Ę'), U'E');
    EXPECT_EQ(UTF::foldAccent(U'ń'), U'n');
    EXPECT_EQ(UTF::foldAccent(U'Ń'), U'N');
    EXPECT_EQ(UTF::foldAccent(U'ó'), U'o');
    EXPECT_EQ(UTF::foldAccent(U'Ó'), U'O');
    EXPECT_EQ(UTF::foldAccent(U'ś'), U's');
    EXPECT_EQ(UTF::foldAccent(U'Ś'), U'S');
    EXPECT_EQ(UTF::foldAccent(U'ź'), U'z');
    EXPECT_EQ(UTF::foldAccent(U'Ź'), U'Z');
    EXPECT_EQ(UTF::foldAccent(U'ż'), U'z');
    EXPECT_EQ(UTF::foldAccent(U'Ż'), U'Z');

    // ł is a separate letter - standard folding should NOT change it
    EXPECT_EQ(UTF::foldAccent(U'ł'), U'ł');
    EXPECT_EQ(UTF::foldAccent(U'Ł'), U'Ł');
}

TEST(AccentFolding, AggressivePolish) {
    // Aggressive folding: ł→l (for search purposes)
    EXPECT_EQ(UTF::foldAccentAggressive(U'ł'), U'l');
    EXPECT_EQ(UTF::foldAccentAggressive(U'Ł'), U'L');

    // Also handles ą→a etc.
    EXPECT_EQ(UTF::foldAccentAggressive(U'ą'), U'a');
}

TEST(AccentFolding, OtherLetters) {
    // German umlauts
    EXPECT_EQ(UTF::foldAccent(U'ä'), U'a');
    EXPECT_EQ(UTF::foldAccent(U'ö'), U'o');
    EXPECT_EQ(UTF::foldAccent(U'ü'), U'u');

    // French accents
    EXPECT_EQ(UTF::foldAccent(U'é'), U'e');
    EXPECT_EQ(UTF::foldAccent(U'è'), U'e');
    EXPECT_EQ(UTF::foldAccent(U'ê'), U'e');
    EXPECT_EQ(UTF::foldAccent(U'ç'), U'c');

    // Nordic letters - these are separate letters (standard fold keeps them)
    EXPECT_EQ(UTF::foldAccent(U'ø'), U'ø');
    EXPECT_EQ(UTF::foldAccent(U'Ø'), U'Ø');

    // Aggressive fold converts them
    EXPECT_EQ(UTF::foldAccentAggressive(U'ø'), U'o');
    EXPECT_EQ(UTF::foldAccentAggressive(U'Ø'), U'O');
    EXPECT_EQ(UTF::foldAccentAggressive(U'đ'), U'd');
    EXPECT_EQ(UTF::foldAccentAggressive(U'Đ'), U'D');
}

TEST(AccentFolding, AggressiveExpand) {
    UTF utf;
    // ß → ss
    u32string str = U"straße";
    u32string folded = utf.foldAccentsAggressive(str);
    EXPECT_EQ(folded, U"strasse");

    // æ → ae
    u32string ae = U"Cæsar";
    u32string ae_folded = utf.foldAccentsAggressive(ae);
    EXPECT_EQ(ae_folded, U"Caesar");

    // œ → oe
    u32string oe = U"cœur";
    u32string oe_folded = utf.foldAccentsAggressive(oe);
    EXPECT_EQ(oe_folded, U"coeur");
}

TEST(AccentFolding, UTF8Convenience) {
    UTF utf;
    // Standard folding
    std::string polishText = "zażółć gęślą jaźń";
    std::string standard = utf.foldAccents8(polishText);
    EXPECT_EQ(standard, "zazołc gesla jazn"); // ł stays ł

    // Aggressive folding
    std::string aggressive = utf.foldAccents8Aggressive(polishText);
    EXPECT_EQ(aggressive, "zazolc gesla jazn"); // ł → l
}

TEST(AccentFolding, NoChange) {
    // ASCII letters should not change
    EXPECT_EQ(UTF::foldAccent(U'a'), U'a');
    EXPECT_EQ(UTF::foldAccent(U'Z'), U'Z');

    // Numbers should not change
    EXPECT_EQ(UTF::foldAccent(U'0'), U'0');
    EXPECT_EQ(UTF::foldAccent(U'9'), U'9');

    // Symbols should not change
    EXPECT_EQ(UTF::foldAccent(U'@'), U'@');
    EXPECT_EQ(UTF::foldAccent(U'!'), U'!');
}
