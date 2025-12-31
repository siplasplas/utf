//
// Created by andrzej on 8/27/22.
//
#include <gtest/gtest.h>
#include "UTF.hpp"

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
