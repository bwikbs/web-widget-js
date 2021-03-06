/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "Escargot.h"
#include "esprima.h"

#include "ast/AST.h"
#include "runtime/StaticStrings.h"
#include "wtfbridge.h"

#include "double-conversion.h"
#include "ieee.h"

using namespace JSC::Yarr;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

namespace esprima {

const char16_t* TokenName[] = {
    u"",
    u"Boolean",
    u"<end>",
    u"Identifier",
    u"Keyword",
    u"Null",
    u"Numeric",
    u"Punctuator",
    u"String",
    u"RegularExpression",
    u"Template",
};

const char16_t* PuncuatorsTokens[] = {
    u"(", u")", u"{", u"}", u".", u"...", u",", u":", u";", u"[", u"]",
    // binary/unary operators
    u"?", u"~", u">>>", u">>", u"<<", u"+", u"-", u"*", u"/", u"%", u"!",
    u"===", u"!==", u"==", u"!=", u"&&", u"||", u"++", u"--", u"&", u"|",
    u"^", u"<", u">", u"in", u"instanceof" ,
    // assignment operators
    u"=", u">>>=", u"<<=", u">>=", u"+=", u"-=", u"*=", u"/=", u"%=",
    u"&=", u"|=", u"^=", u"<=", u">=",
    // SubstitutionEnd
    u"",
    // arrow
    u"=>"
};

const char16_t* KeywordTokens[] = {
    u"", u"if", u"in", u"do", u"var", u"for", u"new", u"try", u"this",
    u"else", u"case", u"void", u"with", u"enum", u"while", u"break",
    u"catch", u"throw", u"const", u"class", u"super", u"return", u"typeof",
    u"delete", u"switch", u"export", u"import", u"default", u"finally",
    u"extends", u"function", u"continue", u"debugger", u"instanceof",
    u"", u"implements", u"interface", u"package", u"private", u"protected", u"public",
    u"static", u"yield", u"let"
};

// TODO handle error

ALWAYS_INLINE bool isDecimalDigit(char16_t ch)
{
    return (ch >= '0' && ch <= '9'); // 0..9
}

ALWAYS_INLINE bool isHexDigit(char16_t ch)
{
    return isDecimalDigit(ch) || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
}

ALWAYS_INLINE bool isOctalDigit(char16_t ch)
{
    return (ch >= '0' && ch < '8'); // 0..7
}

ALWAYS_INLINE uint8_t toHexNumericValue(char16_t ch)
{
    return ch < 'A' ? ch - '0' : (ch - 'A' + 10) & 0xF;
}

// ECMA-262 11.6 Identifier Names and Identifiers
ALWAYS_INLINE char16_t fromCodePoint(char16_t cp)
{
    return cp;
    /*
    if (cp < 0x10000) {
        return cp;
    } else {
        RELEASE_ASSERT_NOT_REACHED();
        // String.fromCharCode(0xD800 + ((cp - 0x10000) >> 10)) +
        // String.fromCharCode(0xDC00 + ((cp - 0x10000) & 1023));
    }
    */
}

bool isIdentifierStartSlow(char16_t ch)
{
    return (ch == 0xAA) || (ch == 0xB5) || (ch == 0xB7) || (ch == 0xBA) || (0xC0 <= ch && ch <= 0xD6) || (0xD8 <= ch && ch <= 0xF6) || (0xF8 <= ch && ch <= 0x02C1) || (0x02C6 <= ch && ch <= 0x02D1) || (0x02E0 <= ch && ch <= 0x02E4) || (ch == 0x02EC) || (ch == 0x02EE) || (0x0300 <= ch && ch <= 0x0374) || (ch == 0x0376) || (ch == 0x0377) || (0x037A <= ch && ch <= 0x037D) || (ch == 0x037F) || (0x0386 <= ch && ch <= 0x038A) || (ch == 0x038C) || (0x038E <= ch && ch <= 0x03A1) || (0x03A3 <= ch && ch <= 0x03F5) || (0x03F7 <= ch && ch <= 0x0481) || (0x0483 <= ch && ch <= 0x0487) || (0x048A <= ch && ch <= 0x052F) || (0x0531 <= ch && ch <= 0x0556) || (ch == 0x0559) || (0x0561 <= ch && ch <= 0x0587) || (0x0591 <= ch && ch <= 0x05BD) || (ch == 0x05BF) || (ch == 0x05C1) || (ch == 0x05C2) || (ch == 0x05C4) || (ch == 0x05C5) || (ch == 0x05C7) || (0x05D0 <= ch && ch <= 0x05EA) || (0x05F0 <= ch && ch <= 0x05F2) || (0x0610 <= ch && ch <= 0x061A) || (0x0620 <= ch && ch <= 0x0669) || (0x066E <= ch && ch <= 0x06D3) || (0x06D5 <= ch && ch <= 0x06DC) || (0x06DF <= ch && ch <= 0x06E8) || (0x06EA <= ch && ch <= 0x06FC) || (ch == 0x06FF) || (0x0710 <= ch && ch <= 0x074A) || (0x074D <= ch && ch <= 0x07B1) || (0x07C0 <= ch && ch <= 0x07F5) || (ch == 0x07FA) || (0x0800 <= ch && ch <= 0x082D) || (0x0840 <= ch && ch <= 0x085B) || (0x08A0 <= ch && ch <= 0x08B2) || (0x08E4 <= ch && ch <= 0x0963) || (0x0966 <= ch && ch <= 0x096F) || (0x0971 <= ch && ch <= 0x0983) || (0x0985 <= ch && ch <= 0x098C) || (ch == 0x098F) || (ch == 0x0990) || (0x0993 <= ch && ch <= 0x09A8) || (0x09AA <= ch && ch <= 0x09B0) || (ch == 0x09B2) || (0x09B6 <= ch && ch <= 0x09B9) || (0x09BC <= ch && ch <= 0x09C4) || (ch == 0x09C7) || (ch == 0x09C8) || (0x09CB <= ch && ch <= 0x09CE) || (ch == 0x09D7) || (ch == 0x09DC) || (ch == 0x09DD) || (0x09DF <= ch && ch <= 0x09E3) || (0x09E6 <= ch && ch <= 0x09F1) || (0x0A01 <= ch && ch <= 0x0A03) || (0x0A05 <= ch && ch <= 0x0A0A) || (ch == 0x0A0F) || (ch == 0x0A10) || (0x0A13 <= ch && ch <= 0x0A28) || (0x0A2A <= ch && ch <= 0x0A30) || (ch == 0x0A32) || (ch == 0x0A33) || (ch == 0x0A35) || (ch == 0x0A36) || (ch == 0x0A38) || (ch == 0x0A39) || (ch == 0x0A3C) || (0x0A3E <= ch && ch <= 0x0A42) || (ch == 0x0A47) || (ch == 0x0A48) || (0x0A4B <= ch && ch <= 0x0A4D) || (ch == 0x0A51) || (0x0A59 <= ch && ch <= 0x0A5C) || (ch == 0x0A5E) || (0x0A66 <= ch && ch <= 0x0A75) || (0x0A81 <= ch && ch <= 0x0A83) || (0x0A85 <= ch && ch <= 0x0A8D) || (0x0A8F <= ch && ch <= 0x0A91) || (0x0A93 <= ch && ch <= 0x0AA8) || (0x0AAA <= ch && ch <= 0x0AB0) || (ch == 0x0AB2) || (ch == 0x0AB3) || (0x0AB5 <= ch && ch <= 0x0AB9) || (0x0ABC <= ch && ch <= 0x0AC5) || (0x0AC7 <= ch && ch <= 0x0AC9) || (0x0ACB <= ch && ch <= 0x0ACD) || (ch == 0x0AD0) || (0x0AE0 <= ch && ch <= 0x0AE3) || (0x0AE6 <= ch && ch <= 0x0AEF) || (0x0B01 <= ch && ch <= 0x0B03) || (0x0B05 <= ch && ch <= 0x0B0C) || (ch == 0x0B0F) || (ch == 0x0B10) || (0x0B13 <= ch && ch <= 0x0B28) || (0x0B2A <= ch && ch <= 0x0B30) || (ch == 0x0B32) || (ch == 0x0B33) || (0x0B35 <= ch && ch <= 0x0B39) || (0x0B3C <= ch && ch <= 0x0B44) || (ch == 0x0B47) || (ch == 0x0B48) || (0x0B4B <= ch && ch <= 0x0B4D) || (ch == 0x0B56) || (ch == 0x0B57) || (ch == 0x0B5C) || (ch == 0x0B5D) || (0x0B5F <= ch && ch <= 0x0B63) || (0x0B66 <= ch && ch <= 0x0B6F) || (ch == 0x0B71) || (ch == 0x0B82) || (ch == 0x0B83) || (0x0B85 <= ch && ch <= 0x0B8A) || (0x0B8E <= ch && ch <= 0x0B90) || (0x0B92 <= ch && ch <= 0x0B95) || (ch == 0x0B99) || (ch == 0x0B9A) || (ch == 0x0B9C) || (ch == 0x0B9E) || (ch == 0x0B9F) || (ch == 0x0BA3) || (ch == 0x0BA4) || (0x0BA8 <= ch && ch <= 0x0BAA) || (0x0BAE <= ch && ch <= 0x0BB9) || (0x0BBE <= ch && ch <= 0x0BC2) || (0x0BC6 <= ch && ch <= 0x0BC8) || (0x0BCA <= ch && ch <= 0x0BCD) || (ch == 0x0BD0) || (ch == 0x0BD7) || (0x0BE6 <= ch && ch <= 0x0BEF) || (0x0C00 <= ch && ch <= 0x0C03) || (0x0C05 <= ch && ch <= 0x0C0C) || (0x0C0E <= ch && ch <= 0x0C10) || (0x0C12 <= ch && ch <= 0x0C28) || (0x0C2A <= ch && ch <= 0x0C39) || (0x0C3D <= ch && ch <= 0x0C44) || (0x0C46 <= ch && ch <= 0x0C48) || (0x0C4A <= ch && ch <= 0x0C4D) || (ch == 0x0C55) || (ch == 0x0C56) || (ch == 0x0C58) || (ch == 0x0C59) || (0x0C60 <= ch && ch <= 0x0C63) || (0x0C66 <= ch && ch <= 0x0C6F) || (0x0C81 <= ch && ch <= 0x0C83) || (0x0C85 <= ch && ch <= 0x0C8C) || (0x0C8E <= ch && ch <= 0x0C90) || (0x0C92 <= ch && ch <= 0x0CA8) || (0x0CAA <= ch && ch <= 0x0CB3) || (0x0CB5 <= ch && ch <= 0x0CB9) || (0x0CBC <= ch && ch <= 0x0CC4) || (0x0CC6 <= ch && ch <= 0x0CC8) || (0x0CCA <= ch && ch <= 0x0CCD) || (ch == 0x0CD5) || (ch == 0x0CD6) || (ch == 0x0CDE) || (0x0CE0 <= ch && ch <= 0x0CE3) || (0x0CE6 <= ch && ch <= 0x0CEF) || (ch == 0x0CF1) || (ch == 0x0CF2) || (0x0D01 <= ch && ch <= 0x0D03) || (0x0D05 <= ch && ch <= 0x0D0C) || (0x0D0E <= ch && ch <= 0x0D10) || (0x0D12 <= ch && ch <= 0x0D3A) || (0x0D3D <= ch && ch <= 0x0D44) || (0x0D46 <= ch && ch <= 0x0D48) || (0x0D4A <= ch && ch <= 0x0D4E) || (ch == 0x0D57) || (0x0D60 <= ch && ch <= 0x0D63) || (0x0D66 <= ch && ch <= 0x0D6F) || (0x0D7A <= ch && ch <= 0x0D7F) || (ch == 0x0D82) || (ch == 0x0D83) || (0x0D85 <= ch && ch <= 0x0D96) || (0x0D9A <= ch && ch <= 0x0DB1) || (0x0DB3 <= ch && ch <= 0x0DBB) || (ch == 0x0DBD) || (0x0DC0 <= ch && ch <= 0x0DC6) || (ch == 0x0DCA) || (0x0DCF <= ch && ch <= 0x0DD4) || (ch == 0x0DD6) || (0x0DD8 <= ch && ch <= 0x0DDF) || (0x0DE6 <= ch && ch <= 0x0DEF) || (ch == 0x0DF2) || (ch == 0x0DF3) || (0x0E01 <= ch && ch <= 0x0E3A) || (0x0E40 <= ch && ch <= 0x0E4E) || (0x0E50 <= ch && ch <= 0x0E59) || (ch == 0x0E81) || (ch == 0x0E82) || (ch == 0x0E84) || (ch == 0x0E87) || (ch == 0x0E88) || (ch == 0x0E8A) || (ch == 0x0E8D) || (0x0E94 <= ch && ch <= 0x0E97) || (0x0E99 <= ch && ch <= 0x0E9F) || (0x0EA1 <= ch && ch <= 0x0EA3) || (ch == 0x0EA5) || (ch == 0x0EA7) || (ch == 0x0EAA) || (ch == 0x0EAB) || (0x0EAD <= ch && ch <= 0x0EB9) || (0x0EBB <= ch && ch <= 0x0EBD) || (0x0EC0 <= ch && ch <= 0x0EC4) || (ch == 0x0EC6) || (0x0EC8 <= ch && ch <= 0x0ECD) || (0x0ED0 <= ch && ch <= 0x0ED9) || (0x0EDC <= ch && ch <= 0x0EDF) || (ch == 0x0F00) || (ch == 0x0F18) || (ch == 0x0F19) || (0x0F20 <= ch && ch <= 0x0F29) || (ch == 0x0F35) || (ch == 0x0F37) || (ch == 0x0F39) || (0x0F3E <= ch && ch <= 0x0F47) || (0x0F49 <= ch && ch <= 0x0F6C) || (0x0F71 <= ch && ch <= 0x0F84) || (0x0F86 <= ch && ch <= 0x0F97) || (0x0F99 <= ch && ch <= 0x0FBC) || (ch == 0x0FC6) || (0x1000 <= ch && ch <= 0x1049) || (0x1050 <= ch && ch <= 0x109D) || (0x10A0 <= ch && ch <= 0x10C5) || (ch == 0x10C7) || (ch == 0x10CD) || (0x10D0 <= ch && ch <= 0x10FA) || (0x10FC <= ch && ch <= 0x1248) || (0x124A <= ch && ch <= 0x124D) || (0x1250 <= ch && ch <= 0x1256) || (ch == 0x1258) || (0x125A <= ch && ch <= 0x125D) || (0x1260 <= ch && ch <= 0x1288) || (0x128A <= ch && ch <= 0x128D) || (0x1290 <= ch && ch <= 0x12B0) || (0x12B2 <= ch && ch <= 0x12B5) || (0x12B8 <= ch && ch <= 0x12BE) || (ch == 0x12C0) || (0x12C2 <= ch && ch <= 0x12C5) || (0x12C8 <= ch && ch <= 0x12D6) || (0x12D8 <= ch && ch <= 0x1310) || (0x1312 <= ch && ch <= 0x1315) || (0x1318 <= ch && ch <= 0x135A) || (0x135D <= ch && ch <= 0x135F) || (0x1369 <= ch && ch <= 0x1371) || (0x1380 <= ch && ch <= 0x138F) || (0x13A0 <= ch && ch <= 0x13F4) || (0x1401 <= ch && ch <= 0x166C) || (0x166F <= ch && ch <= 0x167F) || (0x1681 <= ch && ch <= 0x169A) || (0x16A0 <= ch && ch <= 0x16EA) || (0x16EE <= ch && ch <= 0x16F8) || (0x1700 <= ch && ch <= 0x170C) || (0x170E <= ch && ch <= 0x1714) || (0x1720 <= ch && ch <= 0x1734) || (0x1740 <= ch && ch <= 0x1753) || (0x1760 <= ch && ch <= 0x176C) || (0x176E <= ch && ch <= 0x1770) || (ch == 0x1772) || (ch == 0x1773) || (0x1780 <= ch && ch <= 0x17D3) || (ch == 0x17D7) || (ch == 0x17DC) || (ch == 0x17DD) || (0x17E0 <= ch && ch <= 0x17E9) || (0x180B <= ch && ch <= 0x180D) || (0x1810 <= ch && ch <= 0x1819) || (0x1820 <= ch && ch <= 0x1877) || (0x1880 <= ch && ch <= 0x18AA) || (0x18B0 <= ch && ch <= 0x18F5) || (0x1900 <= ch && ch <= 0x191E) || (0x1920 <= ch && ch <= 0x192B) || (0x1930 <= ch && ch <= 0x193B) || (0x1946 <= ch && ch <= 0x196D) || (0x1970 <= ch && ch <= 0x1974) || (0x1980 <= ch && ch <= 0x19AB) || (0x19B0 <= ch && ch <= 0x19C9) || (0x19D0 <= ch && ch <= 0x19DA) || (0x1A00 <= ch && ch <= 0x1A1B) || (0x1A20 <= ch && ch <= 0x1A5E) || (0x1A60 <= ch && ch <= 0x1A7C) || (0x1A7F <= ch && ch <= 0x1A89) || (0x1A90 <= ch && ch <= 0x1A99) || (ch == 0x1AA7) || (0x1AB0 <= ch && ch <= 0x1ABD) || (0x1B00 <= ch && ch <= 0x1B4B) || (0x1B50 <= ch && ch <= 0x1B59) || (0x1B6B <= ch && ch <= 0x1B73) || (0x1B80 <= ch && ch <= 0x1BF3) || (0x1C00 <= ch && ch <= 0x1C37) || (0x1C40 <= ch && ch <= 0x1C49) || (0x1C4D <= ch && ch <= 0x1C7D) || (0x1CD0 <= ch && ch <= 0x1CD2) || (0x1CD4 <= ch && ch <= 0x1CF6) || (ch == 0x1CF8) || (ch == 0x1CF9) || (0x1D00 <= ch && ch <= 0x1DF5) || (0x1DFC <= ch && ch <= 0x1F15) || (0x1F18 <= ch && ch <= 0x1F1D) || (0x1F20 <= ch && ch <= 0x1F45) || (0x1F48 <= ch && ch <= 0x1F4D) || (0x1F50 <= ch && ch <= 0x1F57) || (ch == 0x1F59) || (ch == 0x1F5B) || (ch == 0x1F5D) || (0x1F5F <= ch && ch <= 0x1F7D) || (0x1F80 <= ch && ch <= 0x1FB4) || (0x1FB6 <= ch && ch <= 0x1FBC) || (ch == 0x1FBE) || (0x1FC2 <= ch && ch <= 0x1FC4) || (0x1FC6 <= ch && ch <= 0x1FCC) || (0x1FD0 <= ch && ch <= 0x1FD3) || (0x1FD6 <= ch && ch <= 0x1FDB) || (0x1FE0 <= ch && ch <= 0x1FEC) || (0x1FF2 <= ch && ch <= 0x1FF4) || (0x1FF6 <= ch && ch <= 0x1FFC) || (ch == 0x200C) || (ch == 0x200D) || (ch == 0x203F) || (ch == 0x2040) || (ch == 0x2054) || (ch == 0x2071) || (ch == 0x207F) || (0x2090 <= ch && ch <= 0x209C) || (0x20D0 <= ch && ch <= 0x20DC) || (ch == 0x20E1) || (0x20E5 <= ch && ch <= 0x20F0) || (ch == 0x2102) || (ch == 0x2107) || (0x210A <= ch && ch <= 0x2113) || (ch == 0x2115) || (0x2118 <= ch && ch <= 0x211D) || (ch == 0x2124) || (ch == 0x2126) || (ch == 0x2128) || (0x212A <= ch && ch <= 0x2139) || (0x213C <= ch && ch <= 0x213F) || (0x2145 <= ch && ch <= 0x2149) || (ch == 0x214E) || (0x2160 <= ch && ch <= 0x2188) || (0x2C00 <= ch && ch <= 0x2C2E) || (0x2C30 <= ch && ch <= 0x2C5E) || (0x2C60 <= ch && ch <= 0x2CE4) || (0x2CEB <= ch && ch <= 0x2CF3) || (0x2D00 <= ch && ch <= 0x2D25) || (ch == 0x2D27) || (ch == 0x2D2D) || (0x2D30 <= ch && ch <= 0x2D67) || (ch == 0x2D6F) || (0x2D7F <= ch && ch <= 0x2D96) || (0x2DA0 <= ch && ch <= 0x2DA6) || (0x2DA8 <= ch && ch <= 0x2DAE) || (0x2DB0 <= ch && ch <= 0x2DB6) || (0x2DB8 <= ch && ch <= 0x2DBE) || (0x2DC0 <= ch && ch <= 0x2DC6) || (0x2DC8 <= ch && ch <= 0x2DCE) || (0x2DD0 <= ch && ch <= 0x2DD6) || (0x2DD8 <= ch && ch <= 0x2DDE) || (0x2DE0 <= ch && ch <= 0x2DFF) || (0x3005 <= ch && ch <= 0x3007) || (0x3021 <= ch && ch <= 0x302F) || (0x3031 <= ch && ch <= 0x3035) || (0x3038 <= ch && ch <= 0x303C) || (0x3041 <= ch && ch <= 0x3096) || (0x3099 <= ch && ch <= 0x309F) || (0x30A1 <= ch && ch <= 0x30FA) || (0x30FC <= ch && ch <= 0x30FF) || (0x3105 <= ch && ch <= 0x312D) || (0x3131 <= ch && ch <= 0x318E) || (0x31A0 <= ch && ch <= 0x31BA) || (0x31F0 <= ch && ch <= 0x31FF) || (0x3400 <= ch && ch <= 0x4DB5) || (0x4E00 <= ch && ch <= 0x9FCC) || (0xA000 <= ch && ch <= 0xA48C) || (0xA4D0 <= ch && ch <= 0xA4FD) || (0xA500 <= ch && ch <= 0xA60C) || (0xA610 <= ch && ch <= 0xA62B) || (0xA640 <= ch && ch <= 0xA66F) || (0xA674 <= ch && ch <= 0xA67D) || (0xA67F <= ch && ch <= 0xA69D) || (0xA69F <= ch && ch <= 0xA6F1) || (0xA717 <= ch && ch <= 0xA71F) || (0xA722 <= ch && ch <= 0xA788) || (0xA78B <= ch && ch <= 0xA78E) || (0xA790 <= ch && ch <= 0xA7AD) || (ch == 0xA7B0) || (ch == 0xA7B1) || (0xA7F7 <= ch && ch <= 0xA827) || (0xA840 <= ch && ch <= 0xA873) || (0xA880 <= ch && ch <= 0xA8C4) || (0xA8D0 <= ch && ch <= 0xA8D9) || (0xA8E0 <= ch && ch <= 0xA8F7) || (ch == 0xA8FB) || (0xA900 <= ch && ch <= 0xA92D) || (0xA930 <= ch && ch <= 0xA953) || (0xA960 <= ch && ch <= 0xA97C) || (0xA980 <= ch && ch <= 0xA9C0) || (0xA9CF <= ch && ch <= 0xA9D9) || (0xA9E0 <= ch && ch <= 0xA9FE) || (0xAA00 <= ch && ch <= 0xAA36) || (0xAA40 <= ch && ch <= 0xAA4D) || (0xAA50 <= ch && ch <= 0xAA59) || (0xAA60 <= ch && ch <= 0xAA76) || (0xAA7A <= ch && ch <= 0xAAC2) || (0xAADB <= ch && ch <= 0xAADD) || (0xAAE0 <= ch && ch <= 0xAAEF) || (0xAAF2 <= ch && ch <= 0xAAF6) || (0xAB01 <= ch && ch <= 0xAB06) || (0xAB09 <= ch && ch <= 0xAB0E) || (0xAB11 <= ch && ch <= 0xAB16) || (0xAB20 <= ch && ch <= 0xAB26) || (0xAB28 <= ch && ch <= 0xAB2E) || (0xAB30 <= ch && ch <= 0xAB5A) || (0xAB5C <= ch && ch <= 0xAB5F) || (ch == 0xAB64) || (ch == 0xAB65) || (0xABC0 <= ch && ch <= 0xABEA) || (ch == 0xABEC) || (ch == 0xABED) || (0xABF0 <= ch && ch <= 0xABF9) || (0xAC00 <= ch && ch <= 0xD7A3) || (0xD7B0 <= ch && ch <= 0xD7C6) || (0xD7CB <= ch && ch <= 0xD7FB) || (0xF900 <= ch && ch <= 0xFA6D) || (0xFA70 <= ch && ch <= 0xFAD9) || (0xFB00 <= ch && ch <= 0xFB06) || (0xFB13 <= ch && ch <= 0xFB17) || (0xFB1D <= ch && ch <= 0xFB28) || (0xFB2A <= ch && ch <= 0xFB36) || (0xFB38 <= ch && ch <= 0xFB3C) || (ch == 0xFB3E) || (ch == 0xFB40) || (ch == 0xFB41) || (ch == 0xFB43) || (ch == 0xFB44) || (0xFB46 <= ch && ch <= 0xFBB1) || (0xFBD3 <= ch && ch <= 0xFD3D) || (0xFD50 <= ch && ch <= 0xFD8F) || (0xFD92 <= ch && ch <= 0xFDC7) || (0xFDF0 <= ch && ch <= 0xFDFB) || (0xFE00 <= ch && ch <= 0xFE0F) || (0xFE20 <= ch && ch <= 0xFE2D) || (ch == 0xFE33) || (ch == 0xFE34) || (0xFE4D <= ch && ch <= 0xFE4F) || (0xFE70 <= ch && ch <= 0xFE74) || (0xFE76 <= ch && ch <= 0xFEFC) || (0xFF10 <= ch && ch <= 0xFF19) || (0xFF21 <= ch && ch <= 0xFF3A) || (ch == 0xFF3F) || (0xFF41 <= ch && ch <= 0xFF5A) || (0xFF66 <= ch && ch <= 0xFFBE) || (0xFFC2 <= ch && ch <= 0xFFC7) || (0xFFCA <= ch && ch <= 0xFFCF) || (0xFFD2 <= ch && ch <= 0xFFD7) || (0xFFDA <= ch && ch <= 0xFFDC);
}

ALWAYS_INLINE bool isIdentifierStart(char16_t ch)
{
    // TODO
    return (ch >= 97 && ch <= 122) // a..z
        || (ch >= 65 && ch <= 90) // A..Z
        || (ch == 36) || (ch == 95) // $ (dollar) and _ (underscore)
        || (ch == 92) // \ (backslash)
        || isIdentifierStartSlow(ch);
}

bool isIdentifierPartSlow(char16_t ch)
{
    return (ch == 0xAA) || (ch == 0xB5) || (ch == 0xB7) || (ch == 0xBA) || (0xC0 <= ch && ch <= 0xD6) || (0xD8 <= ch && ch <= 0xF6) || (0xF8 <= ch && ch <= 0x02C1) || (0x02C6 <= ch && ch <= 0x02D1) || (0x02E0 <= ch && ch <= 0x02E4) || (ch == 0x02EC) || (ch == 0x02EE) || (0x0300 <= ch && ch <= 0x0374) || (ch == 0x0376) || (ch == 0x0377) || (0x037A <= ch && ch <= 0x037D) || (ch == 0x037F) || (0x0386 <= ch && ch <= 0x038A) || (ch == 0x038C) || (0x038E <= ch && ch <= 0x03A1) || (0x03A3 <= ch && ch <= 0x03F5) || (0x03F7 <= ch && ch <= 0x0481) || (0x0483 <= ch && ch <= 0x0487) || (0x048A <= ch && ch <= 0x052F) || (0x0531 <= ch && ch <= 0x0556) || (ch == 0x0559) || (0x0561 <= ch && ch <= 0x0587) || (0x0591 <= ch && ch <= 0x05BD) || (ch == 0x05BF) || (ch == 0x05C1) || (ch == 0x05C2) || (ch == 0x05C4) || (ch == 0x05C5) || (ch == 0x05C7) || (0x05D0 <= ch && ch <= 0x05EA) || (0x05F0 <= ch && ch <= 0x05F2) || (0x0610 <= ch && ch <= 0x061A) || (0x0620 <= ch && ch <= 0x0669) || (0x066E <= ch && ch <= 0x06D3) || (0x06D5 <= ch && ch <= 0x06DC) || (0x06DF <= ch && ch <= 0x06E8) || (0x06EA <= ch && ch <= 0x06FC) || (ch == 0x06FF) || (0x0710 <= ch && ch <= 0x074A) || (0x074D <= ch && ch <= 0x07B1) || (0x07C0 <= ch && ch <= 0x07F5) || (ch == 0x07FA) || (0x0800 <= ch && ch <= 0x082D) || (0x0840 <= ch && ch <= 0x085B) || (0x08A0 <= ch && ch <= 0x08B2) || (0x08E4 <= ch && ch <= 0x0963) || (0x0966 <= ch && ch <= 0x096F) || (0x0971 <= ch && ch <= 0x0983) || (0x0985 <= ch && ch <= 0x098C) || (ch == 0x098F) || (ch == 0x0990) || (0x0993 <= ch && ch <= 0x09A8) || (0x09AA <= ch && ch <= 0x09B0) || (ch == 0x09B2) || (0x09B6 <= ch && ch <= 0x09B9) || (0x09BC <= ch && ch <= 0x09C4) || (ch == 0x09C7) || (ch == 0x09C8) || (0x09CB <= ch && ch <= 0x09CE) || (ch == 0x09D7) || (ch == 0x09DC) || (ch == 0x09DD) || (0x09DF <= ch && ch <= 0x09E3) || (0x09E6 <= ch && ch <= 0x09F1) || (0x0A01 <= ch && ch <= 0x0A03) || (0x0A05 <= ch && ch <= 0x0A0A) || (ch == 0x0A0F) || (ch == 0x0A10) || (0x0A13 <= ch && ch <= 0x0A28) || (0x0A2A <= ch && ch <= 0x0A30) || (ch == 0x0A32) || (ch == 0x0A33) || (ch == 0x0A35) || (ch == 0x0A36) || (ch == 0x0A38) || (ch == 0x0A39) || (ch == 0x0A3C) || (0x0A3E <= ch && ch <= 0x0A42) || (ch == 0x0A47) || (ch == 0x0A48) || (0x0A4B <= ch && ch <= 0x0A4D) || (ch == 0x0A51) || (0x0A59 <= ch && ch <= 0x0A5C) || (ch == 0x0A5E) || (0x0A66 <= ch && ch <= 0x0A75) || (0x0A81 <= ch && ch <= 0x0A83) || (0x0A85 <= ch && ch <= 0x0A8D) || (0x0A8F <= ch && ch <= 0x0A91) || (0x0A93 <= ch && ch <= 0x0AA8) || (0x0AAA <= ch && ch <= 0x0AB0) || (ch == 0x0AB2) || (ch == 0x0AB3) || (0x0AB5 <= ch && ch <= 0x0AB9) || (0x0ABC <= ch && ch <= 0x0AC5) || (0x0AC7 <= ch && ch <= 0x0AC9) || (0x0ACB <= ch && ch <= 0x0ACD) || (ch == 0x0AD0) || (0x0AE0 <= ch && ch <= 0x0AE3) || (0x0AE6 <= ch && ch <= 0x0AEF) || (0x0B01 <= ch && ch <= 0x0B03) || (0x0B05 <= ch && ch <= 0x0B0C) || (ch == 0x0B0F) || (ch == 0x0B10) || (0x0B13 <= ch && ch <= 0x0B28) || (0x0B2A <= ch && ch <= 0x0B30) || (ch == 0x0B32) || (ch == 0x0B33) || (0x0B35 <= ch && ch <= 0x0B39) || (0x0B3C <= ch && ch <= 0x0B44) || (ch == 0x0B47) || (ch == 0x0B48) || (0x0B4B <= ch && ch <= 0x0B4D) || (ch == 0x0B56) || (ch == 0x0B57) || (ch == 0x0B5C) || (ch == 0x0B5D) || (0x0B5F <= ch && ch <= 0x0B63) || (0x0B66 <= ch && ch <= 0x0B6F) || (ch == 0x0B71) || (ch == 0x0B82) || (ch == 0x0B83) || (0x0B85 <= ch && ch <= 0x0B8A) || (0x0B8E <= ch && ch <= 0x0B90) || (0x0B92 <= ch && ch <= 0x0B95) || (ch == 0x0B99) || (ch == 0x0B9A) || (ch == 0x0B9C) || (ch == 0x0B9E) || (ch == 0x0B9F) || (ch == 0x0BA3) || (ch == 0x0BA4) || (0x0BA8 <= ch && ch <= 0x0BAA) || (0x0BAE <= ch && ch <= 0x0BB9) || (0x0BBE <= ch && ch <= 0x0BC2) || (0x0BC6 <= ch && ch <= 0x0BC8) || (0x0BCA <= ch && ch <= 0x0BCD) || (ch == 0x0BD0) || (ch == 0x0BD7) || (0x0BE6 <= ch && ch <= 0x0BEF) || (0x0C00 <= ch && ch <= 0x0C03) || (0x0C05 <= ch && ch <= 0x0C0C) || (0x0C0E <= ch && ch <= 0x0C10) || (0x0C12 <= ch && ch <= 0x0C28) || (0x0C2A <= ch && ch <= 0x0C39) || (0x0C3D <= ch && ch <= 0x0C44) || (0x0C46 <= ch && ch <= 0x0C48) || (0x0C4A <= ch && ch <= 0x0C4D) || (ch == 0x0C55) || (ch == 0x0C56) || (ch == 0x0C58) || (ch == 0x0C59) || (0x0C60 <= ch && ch <= 0x0C63) || (0x0C66 <= ch && ch <= 0x0C6F) || (0x0C81 <= ch && ch <= 0x0C83) || (0x0C85 <= ch && ch <= 0x0C8C) || (0x0C8E <= ch && ch <= 0x0C90) || (0x0C92 <= ch && ch <= 0x0CA8) || (0x0CAA <= ch && ch <= 0x0CB3) || (0x0CB5 <= ch && ch <= 0x0CB9) || (0x0CBC <= ch && ch <= 0x0CC4) || (0x0CC6 <= ch && ch <= 0x0CC8) || (0x0CCA <= ch && ch <= 0x0CCD) || (ch == 0x0CD5) || (ch == 0x0CD6) || (ch == 0x0CDE) || (0x0CE0 <= ch && ch <= 0x0CE3) || (0x0CE6 <= ch && ch <= 0x0CEF) || (ch == 0x0CF1) || (ch == 0x0CF2) || (0x0D01 <= ch && ch <= 0x0D03) || (0x0D05 <= ch && ch <= 0x0D0C) || (0x0D0E <= ch && ch <= 0x0D10) || (0x0D12 <= ch && ch <= 0x0D3A) || (0x0D3D <= ch && ch <= 0x0D44) || (0x0D46 <= ch && ch <= 0x0D48) || (0x0D4A <= ch && ch <= 0x0D4E) || (ch == 0x0D57) || (0x0D60 <= ch && ch <= 0x0D63) || (0x0D66 <= ch && ch <= 0x0D6F) || (0x0D7A <= ch && ch <= 0x0D7F) || (ch == 0x0D82) || (ch == 0x0D83) || (0x0D85 <= ch && ch <= 0x0D96) || (0x0D9A <= ch && ch <= 0x0DB1) || (0x0DB3 <= ch && ch <= 0x0DBB) || (ch == 0x0DBD) || (0x0DC0 <= ch && ch <= 0x0DC6) || (ch == 0x0DCA) || (0x0DCF <= ch && ch <= 0x0DD4) || (ch == 0x0DD6) || (0x0DD8 <= ch && ch <= 0x0DDF) || (0x0DE6 <= ch && ch <= 0x0DEF) || (ch == 0x0DF2) || (ch == 0x0DF3) || (0x0E01 <= ch && ch <= 0x0E3A) || (0x0E40 <= ch && ch <= 0x0E4E) || (0x0E50 <= ch && ch <= 0x0E59) || (ch == 0x0E81) || (ch == 0x0E82) || (ch == 0x0E84) || (ch == 0x0E87) || (ch == 0x0E88) || (ch == 0x0E8A) || (ch == 0x0E8D) || (0x0E94 <= ch && ch <= 0x0E97) || (0x0E99 <= ch && ch <= 0x0E9F) || (0x0EA1 <= ch && ch <= 0x0EA3) || (ch == 0x0EA5) || (ch == 0x0EA7) || (ch == 0x0EAA) || (ch == 0x0EAB) || (0x0EAD <= ch && ch <= 0x0EB9) || (0x0EBB <= ch && ch <= 0x0EBD) || (0x0EC0 <= ch && ch <= 0x0EC4) || (ch == 0x0EC6) || (0x0EC8 <= ch && ch <= 0x0ECD) || (0x0ED0 <= ch && ch <= 0x0ED9) || (0x0EDC <= ch && ch <= 0x0EDF) || (ch == 0x0F00) || (ch == 0x0F18) || (ch == 0x0F19) || (0x0F20 <= ch && ch <= 0x0F29) || (ch == 0x0F35) || (ch == 0x0F37) || (ch == 0x0F39) || (0x0F3E <= ch && ch <= 0x0F47) || (0x0F49 <= ch && ch <= 0x0F6C) || (0x0F71 <= ch && ch <= 0x0F84) || (0x0F86 <= ch && ch <= 0x0F97) || (0x0F99 <= ch && ch <= 0x0FBC) || (ch == 0x0FC6) || (0x1000 <= ch && ch <= 0x1049) || (0x1050 <= ch && ch <= 0x109D) || (0x10A0 <= ch && ch <= 0x10C5) || (ch == 0x10C7) || (ch == 0x10CD) || (0x10D0 <= ch && ch <= 0x10FA) || (0x10FC <= ch && ch <= 0x1248) || (0x124A <= ch && ch <= 0x124D) || (0x1250 <= ch && ch <= 0x1256) || (ch == 0x1258) || (0x125A <= ch && ch <= 0x125D) || (0x1260 <= ch && ch <= 0x1288) || (0x128A <= ch && ch <= 0x128D) || (0x1290 <= ch && ch <= 0x12B0) || (0x12B2 <= ch && ch <= 0x12B5) || (0x12B8 <= ch && ch <= 0x12BE) || (ch == 0x12C0) || (0x12C2 <= ch && ch <= 0x12C5) || (0x12C8 <= ch && ch <= 0x12D6) || (0x12D8 <= ch && ch <= 0x1310) || (0x1312 <= ch && ch <= 0x1315) || (0x1318 <= ch && ch <= 0x135A) || (0x135D <= ch && ch <= 0x135F) || (0x1369 <= ch && ch <= 0x1371) || (0x1380 <= ch && ch <= 0x138F) || (0x13A0 <= ch && ch <= 0x13F4) || (0x1401 <= ch && ch <= 0x166C) || (0x166F <= ch && ch <= 0x167F) || (0x1681 <= ch && ch <= 0x169A) || (0x16A0 <= ch && ch <= 0x16EA) || (0x16EE <= ch && ch <= 0x16F8) || (0x1700 <= ch && ch <= 0x170C) || (0x170E <= ch && ch <= 0x1714) || (0x1720 <= ch && ch <= 0x1734) || (0x1740 <= ch && ch <= 0x1753) || (0x1760 <= ch && ch <= 0x176C) || (0x176E <= ch && ch <= 0x1770) || (ch == 0x1772) || (ch == 0x1773) || (0x1780 <= ch && ch <= 0x17D3) || (ch == 0x17D7) || (ch == 0x17DC) || (ch == 0x17DD) || (0x17E0 <= ch && ch <= 0x17E9) || (0x180B <= ch && ch <= 0x180D) || (0x1810 <= ch && ch <= 0x1819) || (0x1820 <= ch && ch <= 0x1877) || (0x1880 <= ch && ch <= 0x18AA) || (0x18B0 <= ch && ch <= 0x18F5) || (0x1900 <= ch && ch <= 0x191E) || (0x1920 <= ch && ch <= 0x192B) || (0x1930 <= ch && ch <= 0x193B) || (0x1946 <= ch && ch <= 0x196D) || (0x1970 <= ch && ch <= 0x1974) || (0x1980 <= ch && ch <= 0x19AB) || (0x19B0 <= ch && ch <= 0x19C9) || (0x19D0 <= ch && ch <= 0x19DA) || (0x1A00 <= ch && ch <= 0x1A1B) || (0x1A20 <= ch && ch <= 0x1A5E) || (0x1A60 <= ch && ch <= 0x1A7C) || (0x1A7F <= ch && ch <= 0x1A89) || (0x1A90 <= ch && ch <= 0x1A99) || (ch == 0x1AA7) || (0x1AB0 <= ch && ch <= 0x1ABD) || (0x1B00 <= ch && ch <= 0x1B4B) || (0x1B50 <= ch && ch <= 0x1B59) || (0x1B6B <= ch && ch <= 0x1B73) || (0x1B80 <= ch && ch <= 0x1BF3) || (0x1C00 <= ch && ch <= 0x1C37) || (0x1C40 <= ch && ch <= 0x1C49) || (0x1C4D <= ch && ch <= 0x1C7D) || (0x1CD0 <= ch && ch <= 0x1CD2) || (0x1CD4 <= ch && ch <= 0x1CF6) || (ch == 0x1CF8) || (ch == 0x1CF9) || (0x1D00 <= ch && ch <= 0x1DF5) || (0x1DFC <= ch && ch <= 0x1F15) || (0x1F18 <= ch && ch <= 0x1F1D) || (0x1F20 <= ch && ch <= 0x1F45) || (0x1F48 <= ch && ch <= 0x1F4D) || (0x1F50 <= ch && ch <= 0x1F57) || (ch == 0x1F59) || (ch == 0x1F5B) || (ch == 0x1F5D) || (0x1F5F <= ch && ch <= 0x1F7D) || (0x1F80 <= ch && ch <= 0x1FB4) || (0x1FB6 <= ch && ch <= 0x1FBC) || (ch == 0x1FBE) || (0x1FC2 <= ch && ch <= 0x1FC4) || (0x1FC6 <= ch && ch <= 0x1FCC) || (0x1FD0 <= ch && ch <= 0x1FD3) || (0x1FD6 <= ch && ch <= 0x1FDB) || (0x1FE0 <= ch && ch <= 0x1FEC) || (0x1FF2 <= ch && ch <= 0x1FF4) || (0x1FF6 <= ch && ch <= 0x1FFC) || (ch == 0x200C) || (ch == 0x200D) || (ch == 0x203F) || (ch == 0x2040) || (ch == 0x2054) || (ch == 0x2071) || (ch == 0x207F) || (0x2090 <= ch && ch <= 0x209C) || (0x20D0 <= ch && ch <= 0x20DC) || (ch == 0x20E1) || (0x20E5 <= ch && ch <= 0x20F0) || (ch == 0x2102) || (ch == 0x2107) || (0x210A <= ch && ch <= 0x2113) || (ch == 0x2115) || (0x2118 <= ch && ch <= 0x211D) || (ch == 0x2124) || (ch == 0x2126) || (ch == 0x2128) || (0x212A <= ch && ch <= 0x2139) || (0x213C <= ch && ch <= 0x213F) || (0x2145 <= ch && ch <= 0x2149) || (ch == 0x214E) || (0x2160 <= ch && ch <= 0x2188) || (0x2C00 <= ch && ch <= 0x2C2E) || (0x2C30 <= ch && ch <= 0x2C5E) || (0x2C60 <= ch && ch <= 0x2CE4) || (0x2CEB <= ch && ch <= 0x2CF3) || (0x2D00 <= ch && ch <= 0x2D25) || (ch == 0x2D27) || (ch == 0x2D2D) || (0x2D30 <= ch && ch <= 0x2D67) || (ch == 0x2D6F) || (0x2D7F <= ch && ch <= 0x2D96) || (0x2DA0 <= ch && ch <= 0x2DA6) || (0x2DA8 <= ch && ch <= 0x2DAE) || (0x2DB0 <= ch && ch <= 0x2DB6) || (0x2DB8 <= ch && ch <= 0x2DBE) || (0x2DC0 <= ch && ch <= 0x2DC6) || (0x2DC8 <= ch && ch <= 0x2DCE) || (0x2DD0 <= ch && ch <= 0x2DD6) || (0x2DD8 <= ch && ch <= 0x2DDE) || (0x2DE0 <= ch && ch <= 0x2DFF) || (0x3005 <= ch && ch <= 0x3007) || (0x3021 <= ch && ch <= 0x302F) || (0x3031 <= ch && ch <= 0x3035) || (0x3038 <= ch && ch <= 0x303C) || (0x3041 <= ch && ch <= 0x3096) || (0x3099 <= ch && ch <= 0x309F) || (0x30A1 <= ch && ch <= 0x30FA) || (0x30FC <= ch && ch <= 0x30FF) || (0x3105 <= ch && ch <= 0x312D) || (0x3131 <= ch && ch <= 0x318E) || (0x31A0 <= ch && ch <= 0x31BA) || (0x31F0 <= ch && ch <= 0x31FF) || (0x3400 <= ch && ch <= 0x4DB5) || (0x4E00 <= ch && ch <= 0x9FCC) || (0xA000 <= ch && ch <= 0xA48C) || (0xA4D0 <= ch && ch <= 0xA4FD) || (0xA500 <= ch && ch <= 0xA60C) || (0xA610 <= ch && ch <= 0xA62B) || (0xA640 <= ch && ch <= 0xA66F) || (0xA674 <= ch && ch <= 0xA67D) || (0xA67F <= ch && ch <= 0xA69D) || (0xA69F <= ch && ch <= 0xA6F1) || (0xA717 <= ch && ch <= 0xA71F) || (0xA722 <= ch && ch <= 0xA788) || (0xA78B <= ch && ch <= 0xA78E) || (0xA790 <= ch && ch <= 0xA7AD) || (ch == 0xA7B0) || (ch == 0xA7B1) || (0xA7F7 <= ch && ch <= 0xA827) || (0xA840 <= ch && ch <= 0xA873) || (0xA880 <= ch && ch <= 0xA8C4) || (0xA8D0 <= ch && ch <= 0xA8D9) || (0xA8E0 <= ch && ch <= 0xA8F7) || (ch == 0xA8FB) || (0xA900 <= ch && ch <= 0xA92D) || (0xA930 <= ch && ch <= 0xA953) || (0xA960 <= ch && ch <= 0xA97C) || (0xA980 <= ch && ch <= 0xA9C0) || (0xA9CF <= ch && ch <= 0xA9D9) || (0xA9E0 <= ch && ch <= 0xA9FE) || (0xAA00 <= ch && ch <= 0xAA36) || (0xAA40 <= ch && ch <= 0xAA4D) || (0xAA50 <= ch && ch <= 0xAA59) || (0xAA60 <= ch && ch <= 0xAA76) || (0xAA7A <= ch && ch <= 0xAAC2) || (0xAADB <= ch && ch <= 0xAADD) || (0xAAE0 <= ch && ch <= 0xAAEF) || (0xAAF2 <= ch && ch <= 0xAAF6) || (0xAB01 <= ch && ch <= 0xAB06) || (0xAB09 <= ch && ch <= 0xAB0E) || (0xAB11 <= ch && ch <= 0xAB16) || (0xAB20 <= ch && ch <= 0xAB26) || (0xAB28 <= ch && ch <= 0xAB2E) || (0xAB30 <= ch && ch <= 0xAB5A) || (0xAB5C <= ch && ch <= 0xAB5F) || (ch == 0xAB64) || (ch == 0xAB65) || (0xABC0 <= ch && ch <= 0xABEA) || (ch == 0xABEC) || (ch == 0xABED) || (0xABF0 <= ch && ch <= 0xABF9) || (0xAC00 <= ch && ch <= 0xD7A3) || (0xD7B0 <= ch && ch <= 0xD7C6) || (0xD7CB <= ch && ch <= 0xD7FB) || (0xF900 <= ch && ch <= 0xFA6D) || (0xFA70 <= ch && ch <= 0xFAD9) || (0xFB00 <= ch && ch <= 0xFB06) || (0xFB13 <= ch && ch <= 0xFB17) || (0xFB1D <= ch && ch <= 0xFB28) || (0xFB2A <= ch && ch <= 0xFB36) || (0xFB38 <= ch && ch <= 0xFB3C) || (ch == 0xFB3E) || (ch == 0xFB40) || (ch == 0xFB41) || (ch == 0xFB43) || (ch == 0xFB44) || (0xFB46 <= ch && ch <= 0xFBB1) || (0xFBD3 <= ch && ch <= 0xFD3D) || (0xFD50 <= ch && ch <= 0xFD8F) || (0xFD92 <= ch && ch <= 0xFDC7) || (0xFDF0 <= ch && ch <= 0xFDFB) || (0xFE00 <= ch && ch <= 0xFE0F) || (0xFE20 <= ch && ch <= 0xFE2D) || (ch == 0xFE33) || (ch == 0xFE34) || (0xFE4D <= ch && ch <= 0xFE4F) || (0xFE70 <= ch && ch <= 0xFE74) || (0xFE76 <= ch && ch <= 0xFEFC) || (0xFF10 <= ch && ch <= 0xFF19) || (0xFF21 <= ch && ch <= 0xFF3A) || (ch == 0xFF3F) || (0xFF41 <= ch && ch <= 0xFF5A) || (0xFF66 <= ch && ch <= 0xFFBE) || (0xFFC2 <= ch && ch <= 0xFFC7) || (0xFFCA <= ch && ch <= 0xFFCF) || (0xFFD2 <= ch && ch <= 0xFFD7) || (0xFFDA <= ch && ch <= 0xFFDC);
}

ALWAYS_INLINE bool isIdentifierPart(char16_t ch)
{
    // TODO
    return (ch >= 97 && ch <= 122) // a..z
        || (ch >= 65 && ch <= 90) // A..Z
        || (ch >= 48 && ch <= 57) // 0..9
        || (ch == 36) || (ch == 95) // $ (dollar) and _ (underscore)
        || (ch == 92) // \ (backslash)
        || isIdentifierPartSlow(ch);
}

ParserString makeParserString(escargot::ESString* str, size_t start, size_t len)
{
    ParserString ret;
    if (str->isASCIIString()) {
        ret.m_buffer = str->toNullableUTF8String().m_buffer + start;
        ret.m_isASCIIString = true;
    } else {
        ret.m_buffer = str->toNullableUTF16String().m_buffer + start;
        ret.m_isASCIIString = false;
    }
    ret.m_length = len;
    return ret;
}



// ECMA-262 11.6.2.2 Future Reserved Words

ALWAYS_INLINE bool isFutureReservedWord(const ParserString& id)
{
    if (id == u"enum") {
        return true;
    } else if (id == u"export") {
        return true;
    } else if (id == u"import") {
        return true;
    } else if (id == u"super") {
        return true;
    } else {
        return false;
    }
}

ALWAYS_INLINE KeywordKind isStrictModeReservedWord(const ParserString& id)
{
    auto len = id.length();
    if (len == 10) {
        if (id == u"implements") {
            return Implements;
        }
    } else if (len == 9) {
        if (id == u"interface") {
            return Interface;
        } else if (id == u"protected") {
            return Protected;
        }
    } else if (len == 7) {
        if (id == u"package") {
            return Package;
        } else if (id == u"private") {
            return Private;
        }
    } else if (len == 6) {
        if (id == u"public") {
            return Public;
        } else if (id == u"static") {
            return Static;
        }
    } else if (len == 5) {
        if (id == u"yield") {
            return Yield;
        }
    } else if (len == 3) {
        if (id == u"let") {
            return Let;
        }
    }
    return NotKeyword;
}

ALWAYS_INLINE KeywordKind isStrictModeReservedWord(const escargot::InternalAtomicString& id)
{
    if (id == escargot::strings->implements)
        return Implements;
    else if (id == escargot::strings->interface)
        return Interface;
    else if (id == escargot::strings->stringProtected)
        return Protected;
    else if (id == escargot::strings->package)
        return Package;
    else if (id == escargot::strings->stringPrivate)
        return Private;
    else if (id == escargot::strings->stringPublic)
        return Public;
    else if (id == escargot::strings->stringStatic)
        return Static;
    else if (id == escargot::strings->yield)
        return Yield;
    else if (id == escargot::strings->let)
        return Let;
    else
        return NotKeyword;
}

ALWAYS_INLINE bool isRestrictedWord(const ParserString& id)
{
    return id == u"eval" || id == u"arguments";
}

ALWAYS_INLINE bool isRestrictedWord(const escargot::InternalAtomicString& id)
{
    return id == escargot::strings->eval || id == escargot::strings->arguments;
}


// ECMA-262 11.6.2.1 Keywords

ALWAYS_INLINE KeywordKind isKeyword(const ParserString& id)
{
    // 'const' is specialized as Keyword in V8.
    // 'yield' and 'let' are for compatibility with SpiderMonkey and ES.next.
    // Some others are from future reserved words.

    char16_t first = id[0];

    switch (id.length()) {
    case 2:
        if (first == 'i') {
            if (id == u"if") {
                return If;
            } else if (id == u"in") {
                return In;
            }

        } else if (first == 'd' && id == u"do") {
            return Do;
        }
        break;
    case 3:
        if (first == 'v' && id == u"var") {
            return Var;
        } else if (first == 'f' && id == u"for") {
            return For;
        } else if (first == 'n' && id == u"new") {
            return New;
        } else if (first == 't' && id == u"try") {
            return Try;
            /*
        } else if (first == 'l' && id == u"let") {
            return Let;
            */
        }
        break;
    case 4:
        if (first == 't' && id == u"this") {
            return This;
        } else if (first == 'e' && id == u"else") {
            return Else;
        } else if (first == 'c' && id == u"case") {
            return Case;
        } else if (first == 'v' && id == u"void") {
            return Void;
        } else if (first == 'w' && id == u"with") {
            return With;
        } else if (first == 'e' && id == u"enum") {
            return Enum;
        }
        break;
    case 5:
        if (first == 'w' && id == u"while") {
            return While;
        } else if (first == 'b' && id == u"break") {
            return Break;
        } else if (first == 'c') {
            if (id == u"catch") {
                return Catch;
            } else if (id == u"const") {
                return Const;
            } else if (id == u"class") {
                return Class;
            }
        } else if (first == 't' && id == u"throw") {
            return Throw;
            /*
        } else if (first == 'y' && id == u"yield") {
            return Yield;
            */
        } else if (first == 's' && id == u"super") {
            return Super;
        }
        break;
    case 6:
        if (first == 'r' && id == u"return") {
            return Return;
        } else if (first == 't' && id == u"typeof") {
            return Typeof;
        } else if (first == 'd' && id == u"delete") {
            return Delete;
        } else if (first == 's' && id == u"switch") {
            return Switch;
        } else if (first == 'e' && id == u"export") {
            return Export;
        } else if (first == 'i' && id == u"import") {
            return Import;
        }
        break;
    case 7:
        if (first == 'd' && id == u"default") {
            return Default;
        } else if (first == 'f' && id == u"finally") {
            return Finally;
        } else if (first == 'e' && id == u"extends") {
            return Extends;
        }
        break;
    case 8:
        if (first == 'f' && id == u"function") {
            return Function;
        } else if (first == 'c' && id == u"continue") {
            return Continue;
        } else if (first == 'd' && id == u"debugger") {
            return Debugger;
        }
        break;
    case 10:
        if (first == 'i' && id == u"instanceof") {
            return InstanceofKeyword;
        }
        break;
    }

    return NotKeyword;
}

bool isPSMallocInited = false;
#define PS_POOL_SIZE 64
ParseStatus* psPool[PS_POOL_SIZE];
size_t psPoolUsage = 0;

ALWAYS_INLINE ParseStatus* psMalloc()
{
    if (psPoolUsage == 0) {
        return new (malloc(sizeof (ParseStatus)))ParseStatus;
    }
    ParseStatus* ps = psPool[psPoolUsage - 1];
    psPoolUsage--;
    return ps;
}

ALWAYS_INLINE void psFree(void* p)
{
    if (psPoolUsage < PS_POOL_SIZE) {
        psPool[psPoolUsage++] = (ParseStatus *)p;
    } else
        free(p);
}

void* ParseStatus::operator new(size_t size)
{
    return psMalloc();
}

void ParseStatus::operator delete(void* p)
{
    return psFree(p);
}

// VariableDeclaratorNode
void addDeclToCurrentContext(ParseContext* ctx, escargot::VariableDeclarationNode* node)
{
    auto vec = node->declarations();
    escargot::VariableDeclaratorVector newDecl;
    for (unsigned i = 0; i < vec.size(); i++) {
        if (vec[i]->isVariableDeclarator()) {
            RELEASE_ASSERT(((escargot::VariableDeclaratorNode*)vec[i])->id()->isIdentifier());
            newDecl.push_back(new escargot::VariableDeclaratorNode(new escargot::IdentifierNode(((escargot::IdentifierNode *)((escargot::VariableDeclaratorNode*)vec[i])->id())->name()), NULL));
        }
    }
    if (newDecl.size()) {
        escargot::VariableDeclarationNode* newNode = new escargot::VariableDeclarationNode(std::move(newDecl));
        ctx->m_currentBody->insert(ctx->m_currentBody->begin(), newNode);
    }
}

void addDeclToCurrentContext(ParseContext* ctx, escargot::VariableDeclaratorNode* node)
{
    RELEASE_ASSERT(node->id()->isIdentifier());
    ctx->m_currentBody->insert(ctx->m_currentBody->begin(), new escargot::VariableDeclaratorNode(new escargot::IdentifierNode(((escargot::IdentifierNode *)node->id())->name()), NULL));
}

void throwEsprimaException()
{
    throw u"";
}

void throwEsprimaException(RefPtr<ParseStatus> message/*token, message*/)
{
    throw message->m_value.toESString();
}

void throwEsprimaException(escargot::ESString* message)
{
    throw message;
}

void throwEsprimaException(escargot::UTF16String message)
{
    throw escargot::ESString::create(message);
}

void throwEsprimaException(const char16_t* message, ParseContext* ctx, escargot::ESErrorObject::Code code)
{
    ASSERT(ctx);
    throw EsprimaError(ctx->m_lineNumber, escargot::ESString::create(message), code);
}

escargot::UTF16String tokenToString(RefPtr<ParseStatus> token)
{
    if (token->m_type == NumericLiteralToken)
        return escargot::ESUTF16String::create(escargot::dtoa(token->m_valueNumber))->toUTF16String();
    else if (token->m_type == PunctuatorToken)
        return PuncuatorsTokens[token->m_punctuatorsKind];
    else if (token->m_type == Token::KeywordToken)
        return KeywordTokens[token->m_keywordKind];
    else
        return token->m_value.toESString()->toUTF16String();
}

void throwUnexpectedToken(RefPtr<ParseStatus> token, escargot::UTF16String message = u"")
{
    escargot::UTF16String err_msg;
    err_msg.append(u"Unexpected ");
    err_msg.append(TokenName[token->m_type]);
    err_msg.append(u" '");
    err_msg.append(std::move(tokenToString(token)));
    if (escargot::UTF16String(message).length() > 0) {
        err_msg.append(u"\'. ");
        err_msg.append(message);
    } else {
        err_msg.append(u"\'.");
    }
    throwEsprimaException(err_msg);
}

void throwUnexpectedToken(RefPtr<ParseStatus> token, PunctuatorsKind kind, const char16_t* message = u"")
{
    escargot::UTF16String err_msg;
    err_msg.append(u"Expect \'");
    err_msg.append(PuncuatorsTokens[kind]);
    if (escargot::UTF16String(message).length() > 0) {
        err_msg.append(u"\'. ");
        err_msg.append(message);
    } else {
        err_msg.append(u"\'.");
    }
    throwUnexpectedToken(token, err_msg);
}

void throwUnexpectedToken(RefPtr<ParseStatus> token, KeywordKind kind, const char16_t* message = u"")
{
    escargot::UTF16String err_msg;
    err_msg.append(u"Expect \'");
    err_msg.append(KeywordTokens[kind]);
    if (escargot::UTF16String(message).length() > 0) {
        err_msg.append(u"\'. ");
        err_msg.append(message);
    } else {
        err_msg.append(u"\'.");
    }
    throwUnexpectedToken(token, err_msg);
}

void throwUnexpectedToken(RefPtr<ParseStatus> token, Token tokenExpected, const char16_t* message = u"")
{
    escargot::UTF16String err_msg;
    err_msg.append(u"Expect \'");
    err_msg.append(TokenName[tokenExpected]);
    if (escargot::UTF16String(message).length() > 0) {
        err_msg.append(u"\'. ");
        err_msg.append(message);
    } else {
        err_msg.append(u"\'.");
    }
    throwUnexpectedToken(token, err_msg);
}

void throwDuplicateIdentifierError(escargot::InternalAtomicString name, const char16_t* message = u"")
{
    escargot::UTF16String err_msg;
    err_msg.append(u"Cannot declare a parameter named '");
    err_msg.append(name.string()->toUTF16String());
    err_msg.append(u"' in strict mode as it has already been declared.");
    throwEsprimaException(err_msg);
}

void throwRestrictedWordUsedError(escargot::InternalAtomicString name)
{
    escargot::UTF16String err_msg;
    err_msg.append(u"Cannot use a variable named '");
    err_msg.append(name.string()->toUTF16String());
    err_msg.append(u"' in strict mode.");
    throwEsprimaException(err_msg);
}

void throwRestrictedWordUsedError(RefPtr<ParseStatus> token)
{
    throwRestrictedWordUsedError(token->m_value.toInternalAtomicString());
}

void throwOctalLiteralUsedError()
{
    throwEsprimaException(u"Octal literals are not allowed in strict mode.");
}

void tolerateUnexpectedToken(/*token, message*/)
{
    /*
    var error = unexpectedTokenError(token, message);
    if (extra.errors) {
        recordError(error);
    } else {
        throw error;
    }
     */
    throw u"unexpectedTokenError";
}

void tolerateError(const char16_t* error)
{
    throw error;
}

struct OctalToDecimalResult {
    int code;
    bool octal;
};

// OctalEscapeLiterals: http://www.ecma-international.org/ecma-262/5.1/index.html#sec-B.1.2
OctalToDecimalResult octalToDecimal(ParseContext* ctx, char16_t ch)
{
    // \0 is not octal escape sequence
    bool octal = (ch != '0');
    int code = ch - '0';

    if (ctx->m_index == ctx->m_length) {
        // OctalDigit
        // 1..7 => octal
        // 0 => not octal
        return OctalToDecimalResult { code, octal };
    }
    char16_t ch2 = ctx->m_sourceString->charAt(ctx->m_index);
    if ('0' <= ch && ch <= '3') {
        if (isOctalDigit(ch2)) {
            ctx->m_index++;
            code = code * 8 + ch2 - '0';

            if (ctx->m_index + 1 == ctx->m_length) {
                // ZeroToThree OctalDigit
                return OctalToDecimalResult { code, true };
            }
            char16_t ch3 = ctx->m_sourceString->charAt(ctx->m_index);

            if (isOctalDigit(ch3)) {
                // ZeroToThree OctalDigit OctalDigit
                ctx->m_index++;
                code = code * 8 + ch3 - '0';
                return OctalToDecimalResult { code, true };
            } else if (!isDecimalDigit(ch3)) {
                // ZeroToThree OctalDigit [lookahead is not in DecimalDigit]
                return OctalToDecimalResult { code, true };
            } else {
                // ZeroToThree OctalDigit 8..9 (not specified in spec)
                return OctalToDecimalResult { code, true };
            }
        } else if (!isDecimalDigit(ch2)) {
            // ZeroToThree [lookahead is not in DecimalDigit]
            return OctalToDecimalResult { code, octal };
        } else {
            // ZeroToThree 8..9 (not specified in spec)
            return OctalToDecimalResult { code, true };
        }
    } else if ('4' <= ch && ch <= '7') {
        if (isOctalDigit(ch2)) {
            ctx->m_index++;
            code = code * 8 + ch2 - '0';

            // FourToSeven OctalDigit
            return OctalToDecimalResult { code, true };
        } else {
            // FourToSeven notOctalDigit (not specfied in spec)
            return OctalToDecimalResult { code, true };
        }
    } else {
        // OctalLiteral not specified in spec
        return OctalToDecimalResult { code, true };
    }
}

char16_t scanHexEscape(ParseContext* ctx, char16_t prefix)
{
    int i, len, ch, code = 0;

    len = (prefix == 'u') ? 4 : 2;
    for (i = 0; i < len; ++i) {
        if (ctx->m_index < ctx->m_length && isHexDigit(ctx->m_sourceString->charAt(ctx->m_index))) {
            ch = ctx->m_sourceString->charAt(ctx->m_index++);
            int c = 0;
            if (ch >= '0' && ch <= '9') {
                c = ch - '0';
            } else if (ch >= 'a' && ch <= 'f') {
                c = ch - 'a' + 10;
            } else if (ch >= 'A' && ch <= 'F') {
                c = ch - 'A' + 10;
            } else {
                ASSERT_NOT_REACHED();
            }
            code = code * 16 + c;
        } else {
            throwEsprimaException();
        }
    }
    return code;
}


char16_t scanUnicodeCodePointEscape(ParseContext* ctx)
{
    char16_t ch, code;

    ch = ctx->m_sourceString->charAt(ctx->m_index);
    code = 0;

    // At least, one hex digit is required.
    if (ch == '}') {
        throwEsprimaException();
    }

    while (ctx->m_index < ctx->m_length) {
        ch = ctx->m_sourceString->charAt(ctx->m_index++);
        if (!isHexDigit(ch)) {
            break;
        }

        int c = 0;
        if (ch >= '0' && ch <= '9') {
            c = ch - '0';
        } else if (ch >= 'a' && ch <= 'f') {
            c = ch - 'a';
        } else if (ch >= 'A' && ch <= 'F') {
            c = ch - 'A';
        } else {
            ASSERT_NOT_REACHED();
        }

        code = code * 16 + c;
    }

    if (/*code > 0x10FFFF ||*/ ch != '}') {
        throwEsprimaException();
    }

    return fromCodePoint(code);
}

char16_t codePointAt(ParseContext* ctx, size_t i)
{
    char16_t cp, first, second;

    cp = ctx->m_sourceString->charAt(i);
    if (cp >= 0xD800 && cp <= 0xDBFF) {
        second = ctx->m_sourceString->charAt(i + 1);
        if (second >= 0xDC00 && second <= 0xDFFF) {
            first = cp;
            cp = (first - 0xD800) * 0x400 + second - 0xDC00 + 0x10000;
        }
    }

    return cp;
}

ParserString getComplexIdentifier(ParseContext* ctx)
{
    char16_t cp;
    char16_t ch;
    ParserString id;

    cp = codePointAt(ctx, ctx->m_index);
    id = fromCodePoint(cp);
    ctx->m_index += id.length();

    // '\u' (U+005C, U+0075) denotes an escaped character.
    if (cp == 0x5C) {
        if (ctx->m_sourceString->charAt(ctx->m_index) != 0x75) {
            throwEsprimaException();
        }
        ++ctx->m_index;
        if (ctx->m_sourceString->charAt(ctx->m_index) == '{') {
            ++ctx->m_index;
            ch = scanUnicodeCodePointEscape(ctx);
        } else {
            ch = scanHexEscape(ctx, 'u');
            cp = ch;
            if (!ch || ch == '\\' || !isIdentifierStart(cp)) {
                throwEsprimaException();
            }
        }
        id = ch;
    }

    while (ctx->m_index < ctx->m_length) {
        cp = codePointAt(ctx, ctx->m_index);
        if (!isIdentifierPart(cp)) {
            break;
        }
        ch = fromCodePoint(cp);
        id += ch;
        // TODO currently, fromCodePoint returns char16_t
        // index += ch.length;
        ctx->m_index += 1;

        // '\u' (U+005C, U+0075) denotes an escaped character.
        if (cp == 0x5C) {
            // CHECKTHIS id.length() - 1 is right?
            id = id.substr(0, id.length() - 1);
            if (ctx->m_sourceString->charAt(ctx->m_index) != 0x75) {
                throwEsprimaException();
            }
            ++ctx->m_index;
            if (ctx->m_sourceString->charAt(ctx->m_index) == '{') {
                ++ctx->m_index;
                ch = scanUnicodeCodePointEscape(ctx);
            } else {
                ch = scanHexEscape(ctx, 'u');
                // cp = ch.charCodeAt(0);
                cp = ch;
                if (!ch || ch == '\\' || !isIdentifierPart(cp)) {
                    throwEsprimaException();
                }
            }
            id += ch;
        }
    }

    return id;
}

ParserString getIdentifier(ParseContext* ctx)
{
    size_t start;
    char16_t ch;

    start = ctx->m_index++;
    while (ctx->m_index < ctx->m_length) {
        ch = ctx->m_sourceString->charAt(ctx->m_index);
        if (ch == 0x5C) {
            // Blackslash (U+005C) marks Unicode escape sequence.
            ctx->m_index = start;
            return getComplexIdentifier(ctx);
        } else if (ch >= 0xD800 && ch < 0xDFFF) {
            // Need to handle surrogate pairs.
            ctx->m_index = start;
            return getComplexIdentifier(ctx);
        }
        if (isIdentifierPart(ch)) {
            ++ctx->m_index;
        } else {
            break;
        }
    }

    // return ctx->m_source.substr(start, ctx->m_index-start);
    // return ParserString(&ctx->m_source[start], ctx->m_index - start);
    return makeParserString(ctx->m_sourceString, start, ctx->m_index - start);
}

void skipSingleLineComment(ParseContext* ctx, int offset)
{
    size_t start;
    char16_t ch, comment;

    start = ctx->m_index - offset;
    /*
    loc = {
        start: {
            line: lineNumber,
            column: index - lineStart - offset
        }
    };*/

    while (ctx->m_index < ctx->m_length) {
        ch = ctx->m_sourceString->charAt(ctx->m_index);
        ++ctx->m_index;
        if (isLineTerminator(ch)) {
            ctx->m_hasLineTerminator = true;
            /*
            if (extra.comments) {
                comment = source.slice(start + offset, index - 1);
                loc.end = {
                    line: lineNumber,
                    column: index - lineStart - 1
                };
            addComment('Line', comment, start, index - 1, loc);
            }*/
            if (ch == 13 && ctx->m_sourceString->charAt(ctx->m_index) == 10) {
                ++ctx->m_index;
            }
            ++ctx->m_lineNumber;
            ctx->m_lineStart = ctx->m_index;
            return;
        }
    }
    /*
    if (extra.comments) {
        comment = source.slice(start + offset, index);
        loc.end = {
            line: lineNumber,
            column: index - lineStart
        };
        addComment('Line', comment, start, index, loc);
    }
    */
}

void skipMultiLineComment(ParseContext* ctx)
{
    size_t start;
    // , loc,
    char16_t ch;
    // , comment;
    /*
    if (extra.comments) {
        start = index - 2;
        loc = {
            start: {
                line: lineNumber,
                column: index - lineStart - 2
            }
        };
    }
     */
    while (ctx->m_index < ctx->m_length) {
        ch = ctx->m_sourceString->charAt(ctx->m_index);
        if (isLineTerminator(ch)) {
            if (ch == 0x0D && ctx->m_sourceString->charAt(ctx->m_index + 1) == 0x0A) {
                ++ctx->m_index;
            }
            ctx->m_hasLineTerminator = true;
            ++ctx->m_lineNumber;
            ++ctx->m_index;
            ctx->m_lineStart = ctx->m_index;
        } else if (ch == 0x2A) {
            // Block comment ends with '*/'.
            if (ctx->m_sourceString->charAt(ctx->m_index + 1) == 0x2F) {
                ++ctx->m_index;
                ++ctx->m_index;
                /*
                if (extra.comments) {
                    comment = source.slice(start + 2, index - 2);
                    loc.end = {
                        line: lineNumber,
                        column: index - lineStart
                    };
                    addComment('Block', comment, start, index, loc);
                }
                 */
                return;
            }
            ++ctx->m_index;
        } else {
            ++ctx->m_index;
        }
    }

    // Ran off the end of the file - the whole thing is a comment
    /*
    if (extra.comments) {
        loc.end = {
            line: lineNumber,
            column: index - lineStart
        };
        comment = source.slice(start + 2, index);
        addComment('Block', comment, start, index, loc);
    }
    */
    tolerateUnexpectedToken();
}

void skipComment(ParseContext* ctx)
{
    char16_t ch;
    bool start;
    ctx->m_hasLineTerminator = false;

    start = (ctx->m_index == 0);
    while (ctx->m_index < ctx->m_length) {
        ch = ctx->m_sourceString->charAt(ctx->m_index);

        if (isWhiteSpace(ch)) {
            ++ctx->m_index;
        } else if (isLineTerminator(ch)) {
            ctx->m_hasLineTerminator = true;
            ++ctx->m_index;
            if (ch == 0x0D && ctx->m_sourceString->charAt(ctx->m_index) == 0x0A) {
                ++ctx->m_index;
            }
            ++ctx->m_lineNumber;
            ctx->m_lineStart = ctx->m_index;
            start = true;
        } else if (ch == 0x2F) { // U+002F is '/'
            ch = ctx->m_sourceString->charAt(ctx->m_index + 1);
            if (ch == 0x2F) {
                ++ctx->m_index;
                ++ctx->m_index;
                skipSingleLineComment(ctx, 2);
                start = true;
            } else if (ch == 0x2A) { // U+002A is '*'
                ++ctx->m_index;
                ++ctx->m_index;
                skipMultiLineComment(ctx);
            } else {
                break;
            }
        } else if (start && ch == 0x2D) { // U+002D is '-'
            // U+003E is '>'
            if ((ctx->m_sourceString->charAt(ctx->m_index + 1) == 0x2D) && (ctx->m_sourceString->charAt(ctx->m_index + 2) == 0x3E)) {
                // '-->' is a single-line comment
                ctx->m_index += 3;
                skipSingleLineComment(ctx, 3);
            } else {
                break;
            }
        } else if (ch == 0x3C) { // U+003C is '<'
            if (ctx->m_sourceString->charAt(ctx->m_index + 1) == '!' && ctx->m_sourceString->charAt(ctx->m_index + 2) == '-' && ctx->m_sourceString->charAt(ctx->m_index + 3) == '-') {
                ++ctx->m_index; // `<`
                ++ctx->m_index; // `!`
                ++ctx->m_index; // `-`
                ++ctx->m_index; // `-`
                skipSingleLineComment(ctx, 4);
            } else {
                break;
            }
        } else {
            break;
        }
    }
}

PassRefPtr<ParseStatus> scanIdentifier(ParseContext* ctx)
{
    size_t start;
    ParserString id;
    Token type;

    start = ctx->m_index;

    // Backslash (U+005C) starts an escaped character.
    id = (ctx->m_sourceString->charAt(ctx->m_index) == 0x5C) ? getComplexIdentifier(ctx) : getIdentifier(ctx);

    // There is no keyword or literal with only one character.
    // Thus, it must be an identifier.
    KeywordKind kk;
    if (id.length() == 1) {
        type = Token::IdentifierToken;
    } else if ((kk = isKeyword(id)) != NotKeyword) {
        type = Token::KeywordToken;
    } else if (id == u"null") {
        type = Token::NullLiteralToken;
    } else if (id == u"true" || id == u"false") {
        type = Token::BooleanLiteralToken;
    } else if (ctx->m_strict && isStrictModeReservedWord(id)) {
        ParseStatus* ps = new ParseStatus(Token::IdentifierToken, std::move(id), ctx->m_lineNumber, ctx->m_lineStart, start, ctx->m_index);
        throwUnexpectedToken(adoptRef(ps), u"Future reserved word cannot used as Identifier in strict mode");
        RELEASE_ASSERT_NOT_REACHED();
    } else {
        type = Token::IdentifierToken;
    }

    ParseStatus* ps = new ParseStatus(type, std::move(id), ctx->m_lineNumber, ctx->m_lineStart, start, ctx->m_index);
    if (type == KeywordToken) {
        ps->m_keywordKind = kk;
        if (kk == In) {
            ps->m_punctuatorsKind = InPunctuator;
        } else if (kk == InstanceofKeyword) {
            ps->m_punctuatorsKind = InstanceOfPunctuator;
        }
    }
    return adoptRef(ps);
}

// ECMA-262 11.7 Punctuators


PassRefPtr<ParseStatus> scanPunctuator(ParseContext* ctx)
{
    ParseStatus* token = new ParseStatus(Token::PunctuatorToken, ctx->m_lineNumber, ctx->m_lineStart, ctx->m_index, ctx->m_index);
    /*
    token = {
        type: Token.Punctuator,
        value: '',
        lineNumber: lineNumber,
        lineStart: lineStart,
        start: index,
        end: index
    };
     */
    // Check for most common single-character punctuators.
    char16_t ch0 = ctx->m_sourceString->charAt(ctx->m_index), ch1, ch2, ch3;
    // std::UTF16String resultStr;
    // resultStr.reserve(4);
    // resultStr += str;
    switch (ch0) {
    case '(':
        /*
        if (extra.tokenize) {
            extra.openParenToken = extra.tokens.length;
        }
         */
        ++ctx->m_index;
        token->m_punctuatorsKind = LeftParenthesis;
        break;

    case '{':
        /*
        if (extra.tokenize) {
            extra.openCurlyToken = extra.tokens.length;
        }
        */
        ctx->m_curlyStack.push_back(Curly("{\0\0"));
        ++ctx->m_index;
        token->m_punctuatorsKind = LeftBrace;
        break;

    case '.':
        ++ctx->m_index;
        token->m_punctuatorsKind = Period;
        if (ctx->m_sourceString->charAt(ctx->m_index) == '.' && ctx->m_sourceString->charAt(ctx->m_index + 1) == '.') {
            // Spread operator: ...
            ctx->m_index += 2;
            // resultStr = u"...";
            token->m_punctuatorsKind = PeriodPeriodPeriod;
        }
        break;

    case '}':
        ++ctx->m_index;
        ctx->m_curlyStack.pop_back();
        token->m_punctuatorsKind = RightBrace;
        break;
    case ')':
        token->m_punctuatorsKind = RightParenthesis;
        ++ctx->m_index;
        break;
    case ';':
        token->m_punctuatorsKind = SemiColon;
        ++ctx->m_index;
        break;
    case ',':
        token->m_punctuatorsKind = Comma;
        ++ctx->m_index;
        break;
    case '[':
        token->m_punctuatorsKind = LeftSquareBracket;
        ++ctx->m_index;
        break;
    case ']':
        token->m_punctuatorsKind = RightSquareBracket;
        ++ctx->m_index;
        break;
    case ':':
        token->m_punctuatorsKind = Colon;
        ++ctx->m_index;
        break;
    case '?':
        token->m_punctuatorsKind = GuessMark;
        ++ctx->m_index;
        break;
    case '~':
        token->m_punctuatorsKind = Wave;
        ++ctx->m_index;
        break;

    case '>':
        ch1 = ctx->m_sourceString->charAt(ctx->m_index + 1);
        if (ch1 == '>') {
            ch2 = ctx->m_sourceString->charAt(ctx->m_index + 2);
            if (ch2 == '>') {
                ch3 = ctx->m_sourceString->charAt(ctx->m_index + 3);
                if (ch3 == '=') {
                    ctx->m_index += 4;
                    token->m_punctuatorsKind = UnsignedRightShiftEqual;
                } else {
                    token->m_punctuatorsKind = UnsignedRightShift;
                    ctx->m_index += 3;
                }
            } else if (ch2 == '=') {
                token->m_punctuatorsKind = RightShiftEqual;
                ctx->m_index += 3;
            } else {
                token->m_punctuatorsKind = RightShift;
                ctx->m_index += 2;
            }
        } else if (ch1 == '=') {
            token->m_punctuatorsKind = RightInequalityEqual;
            ctx->m_index += 2;
        } else {
            token->m_punctuatorsKind = RightInequality;
            ctx->m_index += 1;
        }
        break;
    case '<':
        ch1 = ctx->m_sourceString->charAt(ctx->m_index + 1);
        if (ch1 == '<') {
            ch2 = ctx->m_sourceString->charAt(ctx->m_index + 2);
            if (ch2 == '=') {
                token->m_punctuatorsKind = LeftShiftEqual;
                ctx->m_index += 3;
            } else {
                token->m_punctuatorsKind = LeftShift;
                ctx->m_index += 2;
            }
        } else if (ch1 == '=') {
            token->m_punctuatorsKind = LeftInequalityEqual;
            ctx->m_index += 2;
        } else {
            token->m_punctuatorsKind = LeftInequality;
            ctx->m_index += 1;
        }
        break;
    case '=':
        ch1 = ctx->m_sourceString->charAt(ctx->m_index + 1);
        if (ch1 == '=') {
            ch2 = ctx->m_sourceString->charAt(ctx->m_index + 2);
            if (ch2 == '=') {
                token->m_punctuatorsKind = StrictEqual;
                ctx->m_index += 3;
            } else {
                token->m_punctuatorsKind = Equal;
                ctx->m_index += 2;
            }
        } else if (ch1 == '>') {
            token->m_punctuatorsKind = Arrow;
            ctx->m_index += 2;
        } else {
            token->m_punctuatorsKind = Substitution;
            ctx->m_index += 1;
        }
        break;
    case '!':
        ch1 = ctx->m_sourceString->charAt(ctx->m_index + 1);
        if (ch1 == '=') {
            ch2 = ctx->m_sourceString->charAt(ctx->m_index + 2);
            if (ch2 == '=') {
                token->m_punctuatorsKind = NotStrictEqual;
                ctx->m_index += 3;
            } else {
                token->m_punctuatorsKind = NotEqual;
                ctx->m_index += 2;
            }
        } else {
            token->m_punctuatorsKind = ExclamationMark;
            ctx->m_index += 1;
        }
        break;
    case '&':
        ch1 = ctx->m_sourceString->charAt(ctx->m_index + 1);
        if (ch1 == '&') {
            token->m_punctuatorsKind = LogicalAnd;
            ctx->m_index += 2;
        } else if (ch1 == '=') {
            token->m_punctuatorsKind = BitwiseAndEqual;
            ctx->m_index += 2;
        } else {
            token->m_punctuatorsKind = BitwiseAnd;
            ctx->m_index += 1;
        }
        break;
    case '|':
        ch1 = ctx->m_sourceString->charAt(ctx->m_index + 1);
        if (ch1 == '|') {
            token->m_punctuatorsKind = LogicalOr;
            ctx->m_index += 2;
        } else if (ch1 == '=') {
            token->m_punctuatorsKind = BitwiseOrEqual;
            ctx->m_index += 2;
        } else {
            token->m_punctuatorsKind = BitwiseOr;
            ctx->m_index += 1;
        }
        break;
    case '^':
        ch1 = ctx->m_sourceString->charAt(ctx->m_index + 1);
        if (ch1 == '=') {
            token->m_punctuatorsKind = BitwiseXorEqual;
            ctx->m_index += 2;
        } else {
            token->m_punctuatorsKind = BitwiseXor;
            ctx->m_index += 1;
        }
        break;
    case '+':
        ch1 = ctx->m_sourceString->charAt(ctx->m_index + 1);
        if (ch1 == '+') {
            token->m_punctuatorsKind = PlusPlus;
            ctx->m_index += 2;
        } else if (ch1 == '=') {
            token->m_punctuatorsKind = PlusEqual;
            ctx->m_index += 2;
        } else {
            token->m_punctuatorsKind = Plus;
            ctx->m_index += 1;
        }
        break;
    case '-':
        ch1 = ctx->m_sourceString->charAt(ctx->m_index + 1);
        if (ch1 == '-') {
            token->m_punctuatorsKind = MinusMinus;
            ctx->m_index += 2;
        } else if (ch1 == '=') {
            token->m_punctuatorsKind = MinusEqual;
            ctx->m_index += 2;
        } else {
            token->m_punctuatorsKind = Minus;
            ctx->m_index += 1;
        }
        break;
    case '*':
        ch1 = ctx->m_sourceString->charAt(ctx->m_index + 1);
        if (ch1 == '=') {
            token->m_punctuatorsKind = MultiplyEqual;
            ctx->m_index += 2;
        } else {
            token->m_punctuatorsKind = Multiply;
            ctx->m_index += 1;
        }
        break;
    case '/':
        ch1 = ctx->m_sourceString->charAt(ctx->m_index + 1);
        if (ch1 == '=') {
            token->m_punctuatorsKind = DivideEqual;
            ctx->m_index += 2;
        } else {
            token->m_punctuatorsKind = Divide;
            ctx->m_index += 1;
        }
        break;
    case '%':
        ch1 = ctx->m_sourceString->charAt(ctx->m_index + 1);
        if (ch1 == '=') {
            token->m_punctuatorsKind = ModEqual;
            ctx->m_index += 2;
        } else {
            token->m_punctuatorsKind = Mod;
            ctx->m_index += 1;
        }
        break;
    default:
        throwEsprimaException(u"Invalid character");
        break;
    }

    if (ctx->m_index == token->m_start) {
        throwEsprimaException();
    }

    token->m_end = ctx->m_index;
    // token->m_value = std::move(resultStr);
    return adoptRef(token);
}

PassRefPtr<ParseStatus> scanStringLiteral(ParseContext* ctx)
{
    ParserString str;
    char16_t quote;
    size_t start;
    char16_t ch, unescaped;
    OctalToDecimalResult octToDec;
    bool octal = false;

    const size_t smallBufferMax = 128;
    char16_t smallBuffer[smallBufferMax + 1];
    size_t smallBufferUsage = 0;
    bool strInited = false;

    quote = ctx->m_sourceString->charAt(ctx->m_index);
    ASSERT((quote == '\'' || quote == '"'));

    start = ctx->m_index;
    ++ctx->m_index;

    while (ctx->m_index < ctx->m_length) {
        if (smallBufferUsage >= smallBufferMax && !strInited) {
            str.assign(&smallBuffer[0], &smallBuffer[smallBufferUsage]);
            strInited = true;
        }

        ch = ctx->m_sourceString->charAt(ctx->m_index++);
        if (ch == quote) {
            quote = '\0';
            break;
        } else if (ch == '\\') {
            ch = ctx->m_sourceString->charAt(ctx->m_index++);
            if (!isLineTerminator(ch)) {
                switch (ch) {
                case 'u':
                case 'x':
                    if (ctx->m_sourceString->charAt(ctx->m_index) == '{') {
                        ++ctx->m_index;
                        if (smallBufferUsage < smallBufferMax) {
                            ASSERT(!strInited);
                            smallBuffer[smallBufferUsage++] = scanUnicodeCodePointEscape(ctx);
                        } else
                            str += scanUnicodeCodePointEscape(ctx);
                    } else {
                        unescaped = scanHexEscape(ctx, ch);

                        if (smallBufferUsage < smallBufferMax) {
                            ASSERT(!strInited);
                            smallBuffer[smallBufferUsage++] = unescaped;
                        } else
                            str += unescaped;
                    }
                    break;
                case 'n':
                    if (smallBufferUsage < smallBufferMax) {
                        ASSERT(!strInited);
                        smallBuffer[smallBufferUsage++] = '\n';
                    } else
                        str += '\n';
                    break;
                case 'r':
                    if (smallBufferUsage < smallBufferMax) {
                        ASSERT(!strInited);
                        smallBuffer[smallBufferUsage++] = '\r';
                    } else
                        str += '\r';
                    break;
                case 't':
                    if (smallBufferUsage < smallBufferMax) {
                        ASSERT(!strInited);
                        smallBuffer[smallBufferUsage++] = '\t';
                    } else
                        str += '\t';
                    break;
                case 'b':
                    if (smallBufferUsage < smallBufferMax) {
                        ASSERT(!strInited);
                        smallBuffer[smallBufferUsage++] = '\b';
                    } else
                        str += '\b';
                    break;
                case 'f':
                    if (smallBufferUsage < smallBufferMax) {
                        ASSERT(!strInited);
                        smallBuffer[smallBufferUsage++] = '\f';
                    } else
                        str += '\f';
                    break;
                case 'v':
                    if (smallBufferUsage < smallBufferMax) {
                        ASSERT(!strInited);
                        smallBuffer[smallBufferUsage++] = '\x0B';
                    } else
                        str += '\x0B';
                    break;
                default:
                    if (isOctalDigit(ch)) {
                        octToDec = octalToDecimal(ctx, ch);
                        octal = octToDec.octal || octal;
                        if (smallBufferUsage < smallBufferMax) {
                            ASSERT(!strInited);
                            smallBuffer[smallBufferUsage++] = octToDec.code;
                        } else
                            str += octToDec.code;
                    } else if (isDecimalDigit(ch)) {
                        octal = true;
                        if (smallBufferUsage < smallBufferMax) {
                            ASSERT(!strInited);
                            smallBuffer[smallBufferUsage++] = ch;
                        } else
                            str += ch;
                    } else {
                        // this code below doesn't work correctly with 2(or more) bytes chars
                        // if (smallBufferUsage < smallBufferMax) {
                        //     ASSERT(!strInited);
                        //     smallBuffer[smallBufferUsage++] = ch;
                        // } else
                        //     str += ch;
                        goto commonASCII;
                    }
                    break;
                }
            } else {
                ++ctx->m_lineNumber;
                if ((ch == '\r' && ctx->m_sourceString->charAt(ctx->m_index) == '\n')
                    || (ch == '\n' && ctx->m_sourceString->charAt(ctx->m_index) == '\r')) {
                    ++ctx->m_index;
                }
                ctx->m_lineStart = ctx->m_index;
            }
        } else if (isLineTerminator(ch)) {
            break;
        } else {
commonASCII:
            if (ctx->m_sourceString->isASCIIString()) {
                // NOTE this is workaround for support utf-8
                // ASCII byte
                if (0 == (ch & 0x80)) {

                } else {
                    // Start byte for 2byte
                    char16_t uc = 0;
                    char16_t ch2 = ctx->m_sourceString->charAt(ctx->m_index++);
                    if (0xC0 == (ch & 0xE0)
                        && 0x80 == (ch2 & 0xC0) ) {
                        uc += (ch & 0x1F) << 6;
                        uc += (ch2 & 0x3F) << 0;
                        ch = uc;
                    } else { // Start byte for 3byte
                        char16_t ch3 = ctx->m_sourceString->charAt(ctx->m_index++);
                        if (0xE0 == (ch & 0xF0)
                            && 0x80 == (ch2 & 0xC0)
                            && 0x80 == (ch3 & 0xC0)) {
                            uc += (ch & 0x1F) << 12;
                            uc += (ch2 & 0x3F) << 6;
                            uc += (ch3 & 0x3F) << 0;
                            ch = uc;
                        } else { // support 4-byte case
                            char16_t ch4 = ctx->m_sourceString->charAt(ctx->m_index++);
                            if (0xF0 == (ch & 0xF8)
                                && 0x80 == (ch2 & 0xC0)
                                && 0x80 == (ch3 & 0xC0)
                                && 0x80 == (ch4 & 0xC0)) {
                                char16_t u = (ch & 0x07) << 2 | (ch2 & 0x30) >> 4;
                                char16_t uc0 = 0;
                                uc0 += 0xD8 << 8;
                                uc0 += (u-1) << 6;
                                uc0 += (ch2 & 0x0F) << 2;
                                uc0 += (ch3 & 0x30) >> 4;

                                if (smallBufferUsage < smallBufferMax) {
                                    ASSERT(!strInited);
                                    smallBuffer[smallBufferUsage++] = uc0;
                                } else {
                                    str += uc0;
                                }

                                uc += 0xDC << 8;
                                uc += (ch3 & 0x0F) << 6;
                                uc += (ch4 & 0x3F) << 0;
                                ch = uc;
                            } else { // invalid case
                                RELEASE_ASSERT_NOT_REACHED();
                            }
                        }
                    }
                }
            }
            if (smallBufferUsage < smallBufferMax) {
                ASSERT(!strInited);
                smallBuffer[smallBufferUsage++] = ch;
            } else
                str += ch;
        }
    }

    if (!strInited) {
        str.assign(&smallBuffer[0], &smallBuffer[smallBufferUsage]);
    }
    if (quote != '\0') {
        throwEsprimaException(u"unterminated string literal");
    }

    ParseStatus* ps =  new ParseStatus(
        Token::StringLiteralToken,
        std::move(str),
        octal,
        ctx->m_startLineNumber,
        ctx->m_startLineStart,
        start,
        ctx->m_index
    );
    return adoptRef(ps);
}

// ECMA-262 11.8.6 Template Literal Lexical Components
PassRefPtr<ParseStatus> scanTemplate(ParseContext* ctx)
{
    // var cooked = '', ch, start, rawOffset, terminated, head, tail, restore, unescaped;
    char16_t ch, unescaped;
    bool terminated = false;
    bool tail = false;
    size_t start = ctx->m_index;
    bool head = (ctx->m_sourceString->charAt(ctx->m_index) == '`');
    size_t rawOffset = 2;
    size_t restore;
    ParserStringStd cooked;

    ++ctx->m_index;

    while (ctx->m_index < ctx->m_length) {
        ch = ctx->m_sourceString->charAt(ctx->m_index++);
        if (ch == '`') {
            rawOffset = 1;
            tail = true;
            terminated = true;
            break;
        } else if (ch == '$') {
            if (ctx->m_sourceString->charAt(ctx->m_index) == '{') {
                ctx->m_curlyStack.push_back(Curly("${\0"));
                ++ctx->m_index;
                terminated = true;
                break;
            }
            cooked += ch;
        } else if (ch == '\\') {
            ch = ctx->m_sourceString->charAt(ctx->m_index++);
            if (!isLineTerminator(ch)) {
                switch (ch) {
                case 'n':
                    cooked += '\n';
                    break;
                case 'r':
                    cooked += '\r';
                    break;
                case 't':
                    cooked += '\t';
                    break;
                case 'u':
                case 'x':
                    if (ctx->m_sourceString->charAt(ctx->m_index) == '{') {
                        ++ctx->m_index;
                        cooked += scanUnicodeCodePointEscape(ctx);
                    } else {
                        restore = ctx->m_index;
                        unescaped = scanHexEscape(ctx, ch);
                        if (unescaped) {
                            cooked += unescaped;
                        } else {
                            ctx->m_index = restore;
                            cooked += ch;
                        }
                    }
                    break;
                case 'b':
                    cooked += '\b';
                    break;
                case 'f':
                    cooked += '\f';
                    break;
                case 'v':
                    cooked += '\v';
                    break;

                default:
                    if (ch == '0') {
                        if (isDecimalDigit(ctx->m_sourceString->charAt(ctx->m_index))) {
                            // Illegal: \01 \02 and so on
                            throw u"TemplateOctalLiteral";
                            // throwError(Messages.TemplateOctalLiteral);
                        }
                        cooked.push_back('\0');
                    } else if (isOctalDigit(ch)) {
                        // Illegal: \1 \2
                        // throwError(Messages.TemplateOctalLiteral);
                        throw u"TemplateOctalLiteral";
                    } else {
                        cooked += ch;
                    }
                    break;
                }
            } else {
                ++ctx->m_lineNumber;
                if (ch == '\r' && ctx->m_sourceString->charAt(ctx->m_index) == '\n') {
                    ++ctx->m_index;
                }
                ctx->m_lineStart = ctx->m_index;
            }
        } else if (isLineTerminator(ch)) {
            ++ctx->m_lineNumber;
            if (ch == '\r' && ctx->m_sourceString->charAt(ctx->m_index) == '\n') {
                ++ctx->m_index;
            }
            ctx->m_lineStart = ctx->m_index;
            cooked += '\n';
        } else {
            cooked += ch;
        }
    }

    if (!terminated) {
        throwEsprimaException();
    }

    if (!head) {
        ctx->m_curlyStack.pop_back();
    }

    ParseStatus* status;
    status = new ParseStatus();
    status->m_type = Token::TemplateToken;
    status->m_value = cooked;
    // status->m_value_cooked = cooked;
    // status->m_value_raw = ctx->m_source.substr(start + 1, ctx->m_index - rawOffset - start + 1);
    status->m_head = head;
    status->m_tail = tail;
    status->m_lineNumber = ctx->m_lineNumber;
    status->m_lineStart = ctx->m_lineStart;
    status->m_start = start;
    status->m_end = ctx->m_index;
    return adoptRef(status);
    /*
    return {
        type: Token.Template,
        value: {
            cooked: cooked,
            raw: source.slice(start + 1, index - rawOffset)
        },
        head: head,
        tail: tail,
        lineNumber: lineNumber,
        lineStart: lineStart,
        start: start,
        end: index
    };
    */
}

PassRefPtr<ParseStatus> scanHexLiteral(ParseContext* ctx, size_t start)
{
    bool scanned = false;
    uint64_t number = 0;
    double numberDouble = 0.0;
    bool shouldUseDouble = false;

    size_t shiftCount = 0;
    while (ctx->m_index < ctx->m_length) {
        char16_t ch = ctx->m_sourceString->charAt(ctx->m_index);
        if (!isHexDigit(ch)) {
            break;
        }
        if (shouldUseDouble) {
            numberDouble = numberDouble * 16 + toHexNumericValue(ch);
        } else {
            number = (number << 4) + toHexNumericValue(ch);
            if (++shiftCount >= 16) {
                shouldUseDouble = true;
                numberDouble = number;
                number = 0;
            }
        }
        ctx->m_index++;
        scanned = true;
    }

    if (!scanned) {
        throwEsprimaException();
    }

    if (isIdentifierStart(ctx->m_sourceString->charAt(ctx->m_index))) {
        throwEsprimaException();
    }

    ParseStatus* ps = new ParseStatus;
    ps->m_type = Token::NumericLiteralToken;
    if (shouldUseDouble) {
        ASSERT(number == 0);
        ps->m_valueNumber = numberDouble;
    } else {
        ASSERT(numberDouble == 0.0);
        ps->m_valueNumber = number;
    }
    ps->m_lineNumber = ctx->m_lineNumber;
    ps->m_lineStart = ctx->m_lineStart;
    ps->m_start = start;
    ps->m_end = ctx->m_index;

    /*
    return {
        type: Token.NumericLiteral,
        value: parseInt('0x' + number, 16),
        lineNumber: lineNumber,
        lineStart: lineStart,
        start: start,
        end: index
    };
     */
    return adoptRef(ps);
}

PassRefPtr<ParseStatus> scanBinaryLiteral(ParseContext* ctx, size_t start)
{
    uint64_t number = 0;
    bool scanned = false;

    while (ctx->m_index < ctx->m_length) {
        char16_t ch = ctx->m_sourceString->charAt(ctx->m_index);
        if (ch != '0' && ch != '1') {
            break;
        }
        number = (number << 1) +  ch - '0';
        ctx->m_index++;
        scanned = true;
    }

    if (!scanned) {
        // only 0b or 0B
        throwEsprimaException();
    }

    if (ctx->m_index < ctx->m_length) {
        char16_t ch = ctx->m_sourceString->charAt(ctx->m_index);
        /* istanbul ignore else */
        if (isIdentifierStart(ch) || isDecimalDigit(ch)) {
            throwEsprimaException();
        }
    }

    ParseStatus* ps = new ParseStatus;
    ps->m_type = Token::NumericLiteralToken;
    ps->m_valueNumber = number;
    ps->m_lineNumber = ctx->m_lineNumber;
    ps->m_lineStart = ctx->m_lineStart;
    ps->m_start = start;
    ps->m_end = ctx->m_index;
    return adoptRef(ps);
    /*
    return {
        type: Token.NumericLiteral,
        value: parseInt(number, 2),
        lineNumber: lineNumber,
        lineStart: lineStart,
        start: start,
        end: index
    };
    */
}

PassRefPtr<ParseStatus> scanOctalLiteral(ParseContext* ctx, char16_t prefix, size_t start)
{
    if (ctx->m_strict)
        throwEsprimaException(u"Octal numeric literals and escape characters not allowed in strict mode");

    uint64_t number = 0;
    bool scanned = false;
    bool octal = isOctalDigit(prefix);

    while (ctx->m_index < ctx->m_length) {
        char16_t ch = ctx->m_sourceString->charAt(ctx->m_index);
        if (!isOctalDigit(ch)) {
            break;
        }
        number = (number << 3) + ch - '0';
        ctx->m_index++;
        scanned = true;
    }

    if (!octal && !scanned) {
        // only 0o or 0O
        throwEsprimaException();
    }

    if (isIdentifierStart(ctx->m_sourceString->charAt(ctx->m_index)) || isDecimalDigit(ctx->m_sourceString->charAt(ctx->m_index))) {
        throwEsprimaException();
    }

    ParseStatus* ps = new ParseStatus;
    ps->m_type = Token::NumericLiteralToken;
    ps->m_octal = true;
    ps->m_valueNumber = number;
    ps->m_lineNumber = ctx->m_lineNumber;
    ps->m_lineStart = ctx->m_lineStart;
    ps->m_start = start;
    ps->m_end = ctx->m_index;
    return adoptRef(ps);
    /*
    return {
        type: Token.NumericLiteral,
        value: parseInt(number, 8),
        octal: octal,
        lineNumber: lineNumber,
        lineStart: lineStart,
        start: start,
        end: index
    };
     */
}

bool isImplicitOctalLiteral(ParseContext* ctx)
{
    size_t i;
    char16_t ch;

    // Implicit octal, unless there is a non-octal digit.
    // (Annex B.1.1 on Numeric Literals)
    for (size_t i = ctx->m_index + 1; i < ctx->m_length; ++i) {
        ch = ctx->m_sourceString->charAt(i);
        if (ch == '8' || ch == '9') {
            return false;
        }
        if (!isOctalDigit(ch)) {
            return true;
        }
    }

    return true;
}

PassRefPtr<ParseStatus> scanNumericLiteral(ParseContext* ctx)
{
    std::string number;
    number.reserve(32);
    size_t start;
    char16_t ch;

    ch = ctx->m_sourceString->charAt(ctx->m_index);
    ASSERT(isDecimalDigit(ch) || (ch == '.'));

    start = ctx->m_index;

    if (ch != '.') {
        number = ctx->m_sourceString->charAt(ctx->m_index++);
        ch = ctx->m_sourceString->charAt(ctx->m_index);

        // Hex number starts with '0x'.
        // Octal number starts with '0'.
        // Octal number in ES6 starts with '0o'.
        // Binary number in ES6 starts with '0b'.
        if (number[0] == '0') {
            if (ch == 'x' || ch == 'X') {
                ++ctx->m_index;
                return scanHexLiteral(ctx, start);
            }
            if (ch == 'b' || ch == 'B') {
                ++ctx->m_index;
                return scanBinaryLiteral(ctx, start);
            }
            if (ch == 'o' || ch == 'O') {
                return scanOctalLiteral(ctx, ch, start);
            }

            if (isOctalDigit(ch)) {
                if (isImplicitOctalLiteral(ctx)) {
                    return scanOctalLiteral(ctx, ch, start);
                }
            }
        }

        while (isDecimalDigit(ctx->m_sourceString->charAt(ctx->m_index))) {
            number += ctx->m_sourceString->charAt(ctx->m_index++);
        }
        ch = ctx->m_sourceString->charAt(ctx->m_index);
    }

    if (ch == '.') {
        number += ctx->m_sourceString->charAt(ctx->m_index++);
        while (isDecimalDigit(ctx->m_sourceString->charAt(ctx->m_index))) {
            number += ctx->m_sourceString->charAt(ctx->m_index++);
        }
        ch = ctx->m_sourceString->charAt(ctx->m_index);
    }

    if (ch == 'e' || ch == 'E') {
        number += ctx->m_sourceString->charAt(ctx->m_index++);

        ch = ctx->m_sourceString->charAt(ctx->m_index);
        if (ch == '+' || ch == '-') {
            number += ctx->m_sourceString->charAt(ctx->m_index++);
        }
        if (isDecimalDigit(ctx->m_sourceString->charAt(ctx->m_index))) {
            while (isDecimalDigit(ctx->m_sourceString->charAt(ctx->m_index))) {
                number += ctx->m_sourceString->charAt(ctx->m_index++);
            }
        } else {
            throwEsprimaException();
        }
    }

    if (isIdentifierStart(ctx->m_sourceString->charAt(ctx->m_index))) {
        throwEsprimaException();
    }

    int length = number.length();
    int length_dummy;
    double_conversion::StringToDoubleConverter converter(double_conversion::StringToDoubleConverter::ALLOW_HEX
        | double_conversion::StringToDoubleConverter::ALLOW_LEADING_SPACES
        | double_conversion::StringToDoubleConverter::ALLOW_TRAILING_SPACES, 0.0, double_conversion::Double::NaN(),
        escargot::strings->Infinity.string()->asciiData(), escargot::strings->NaN.string()->asciiData());
    double ll = converter.StringToDouble(number.data(), length, &length_dummy);

    ParseStatus* ps = new ParseStatus;
    ps->m_type = Token::NumericLiteralToken;
    // ps->m_value = number;
    ps->m_valueNumber = ll;
    ps->m_lineNumber = ctx->m_lineNumber;
    ps->m_lineStart = ctx->m_lineStart;
    ps->m_start = start;
    ps->m_end = ctx->m_index;
    return adoptRef(ps);
    /*
    return {
        type: Token.NumericLiteral,
        value: parseFloat(number),
        lineNumber: lineNumber,
        lineStart: lineStart,
        start: start,
        end: index
    };
     */
}


ALWAYS_INLINE PassRefPtr<ParseStatus> advance(ParseContext* ctx)
{
    char16_t cp;

    if (ctx->m_index >= ctx->m_length) {
        /*
        return {
            type: Token.EOF,
            lineNumber: lineNumber,
            lineStart: lineStart,
            start: index,
            end: index
        };
         */
        ParseStatus* ps = new ParseStatus(Token::EOFToken, ctx->m_lineNumber, ctx->m_lineStart, ctx->m_index, ctx->m_index);
        return adoptRef(ps);
    }

    cp = ctx->m_sourceString->charAt(ctx->m_index);

    if (isIdentifierStart(cp)) {
        RefPtr<ParseStatus> token;
        token = scanIdentifier(ctx);
        KeywordKind kk;
        if (ctx->m_strict && (kk = isStrictModeReservedWord(token->m_value)) != NotKeyword) {
            token->m_type = Token::KeywordToken;
            token->m_keywordKind = kk;
        }
        return token;
    }

    // Very common: ( and ) and ;
    if (cp == 0x28 || cp == 0x29 || cp == 0x3B) {
        return scanPunctuator(ctx);
    }

    // String literal starts with single quote (U+0027) or double quote (U+0022).
    if (cp == 0x27 || cp == 0x22) {
        return scanStringLiteral(ctx);
    }

    // Dot (.) U+002E can also start a floating-point number, hence the need
    // to check the next character.
    if (cp == 0x2E) {
        if (isDecimalDigit(ctx->m_sourceString->charAt(ctx->m_index + 1))) {
            return scanNumericLiteral(ctx);
        }
        return scanPunctuator(ctx);
    }

    if (isDecimalDigit(cp)) {
        return scanNumericLiteral(ctx);
    }
    /*
    // Slash (/) U+002F can also start a regex.
    if (extra.tokenize && cp === 0x2F) {
        return advanceSlash();
    }
     */
    // Template literals start with ` (U+0060) for template head
    // or } (U+007D) for template middle or template tail.
    if (cp == 0x7D && ctx->m_curlyStack.size() < 1) {
        throwEsprimaException();
    }
    if (cp == 0x60 || (cp == 0x7D && strcmp(ctx->m_curlyStack[ctx->m_curlyStack.size() - 1].m_curly, "${") == 0)) {
        return scanTemplate(ctx);
    }

    // Possible identifier start in a surrogate pair.
    if (cp >= 0xD800 && cp < 0xDFFF) {
        cp = codePointAt(ctx, ctx->m_index);
        if (isIdentifierStart(cp)) {
            return scanIdentifier(ctx);
        }
    }

    return scanPunctuator(ctx);
}

void peek(ParseContext* ctx)
{
    ctx->m_scanning = true;

    skipComment(ctx);

    ctx->m_lastIndex = ctx->m_index;
    ctx->m_lastLineNumber = ctx->m_lineNumber;
    ctx->m_lastLineStart = ctx->m_lineStart;

    ctx->m_startIndex = ctx->m_index;
    ctx->m_startLineNumber = ctx->m_lineNumber;
    ctx->m_startLineStart = ctx->m_lineStart;

    // lookahead = (typeof extra.tokens !== 'undefined') ? collectToken() : advance();
    ctx->m_lookahead = advance(ctx);
    ctx->m_scanning = false;
}

PassRefPtr<ParseStatus> lex(ParseContext* ctx)
{
    RefPtr<ParseStatus> token;
    ctx->m_scanning = true;

    ctx->m_lastIndex = ctx->m_index;
    ctx->m_lastLineNumber = ctx->m_lineNumber;
    ctx->m_lastLineStart = ctx->m_lineStart;

    skipComment(ctx);

    token = ctx->m_lookahead;

    ctx->m_startIndex = ctx->m_index;
    ctx->m_startLineNumber = ctx->m_lineNumber;
    ctx->m_startLineStart = ctx->m_lineStart;

    // lookahead = (typeof extra.tokens !== 'undefined') ? collectToken() : advance();
    ctx->m_lookahead = advance(ctx);
    ctx->m_scanning = false;
    return token;
}

// Expect the next token to match the specified punctuator.
// If not, an exception will be thrown.
ALWAYS_INLINE void expect(ParseContext* ctx, PunctuatorsKind kind)
{
    RefPtr<ParseStatus> token = lex(ctx);
    // CHECKTHIS. compare value!
    if (token->m_type != Token::PunctuatorToken || token->m_punctuatorsKind != kind) {
        throwUnexpectedToken(token, kind);
    }
}

/**
 * @name expectCommaSeparator
 * @description Quietly expect a comma when in tolerant mode, otherwise delegates
 * to <code>expect(value)</code>
 * @since 2.0
 */
void expectCommaSeparator(ParseContext* ctx)
{
    /*
    ParseStatus* token;
    if (extra.errors) {
        token = lookahead;
        if (token.type === Token.Punctuator && token.value === ',') {
            lex();
        } else if (token.type === Token.Punctuator && token.value === ';') {
            lex();
            tolerateUnexpectedToken(token);
        } else {
            tolerateUnexpectedToken(token, Messages.UnexpectedToken);
        }
    } else {
        expect(',');
    }
     */
    // expect(ctx, Comma);
    expect(ctx, Comma);
}

// Expect the next token to match the specified keyword.
// If not, an exception will be thrown.
void expectKeyword(ParseContext* ctx, KeywordKind keyword)
{
    RefPtr<ParseStatus> token = lex(ctx);
    if (token->m_type != Token::KeywordToken || token->m_keywordKind != keyword) {
        throwUnexpectedToken(token, keyword);
    }
}

// Return true if the next token matches the specified punctuator.
ALWAYS_INLINE bool match(ParseContext* ctx, PunctuatorsKind kind)
{
    return ctx->m_lookahead->m_type == Token::PunctuatorToken && ctx->m_lookahead->m_punctuatorsKind == kind;
}

// Return true if the next token matches the specified keyword
bool matchKeyword(ParseContext* ctx, KeywordKind kk)
{
    return ctx->m_lookahead->m_type == Token::KeywordToken && ctx->m_lookahead->m_keywordKind == kk;
}

// Return true if the next token matches the specified contextual keyword
// (where an identifier is sometimes a keyword depending on the context)

bool matchContextualKeyword(ParseContext* ctx, const ParserString& keyword)
{
    return ctx->m_lookahead->m_type == Token::IdentifierToken && ctx->m_lookahead->m_value == keyword;
}

bool matchContextualKeyword(ParseContext* ctx, const char16_t* keyword)
{
    return ctx->m_lookahead->m_type == Token::IdentifierToken && ctx->m_lookahead->m_value == keyword;
}

// Return true if the next token is an assignment operator

bool matchAssign(ParseContext* ctx)
{
    if (ctx->m_lookahead->m_type != Token::PunctuatorToken) {
        return false;
    }
    const ParserString& op= ctx->m_lookahead->m_value;

    if (ctx->m_lookahead->m_punctuatorsKind >= Substitution && ctx->m_lookahead->m_punctuatorsKind < SubstitutionEnd) {
        return true;
    }

    return false;
}

void consumeSemicolon(ParseContext* ctx)
{
    // Catch the very common case first: immediately a semicolon (U+003B).
    if (ctx->m_sourceString->charAt(ctx->m_startIndex) == 0x3B || match(ctx, SemiColon)) {
        lex(ctx);
        return;
    }

    if (ctx->m_hasLineTerminator) {
        return;
    }

    // FIXME(ikarienator): this is seemingly an issue in the previous location info convention.
    ctx->m_lastIndex = ctx->m_startIndex;
    ctx->m_lastLineNumber = ctx->m_startLineNumber;
    ctx->m_lastLineStart = ctx->m_startLineStart;

    if (ctx->m_lookahead->m_type != Token::EOFToken && !match(ctx, RightBrace)) {
        throwUnexpectedToken(ctx->m_lookahead, u"Expected ';'");
    }
}

#define PARSER_STACK_CHECK ParserStackChecker __checker(ctx)

ParserStackChecker::ParserStackChecker(ParseContext* ctx)
    : m_ctx(ctx)
{
    m_ctx->m_stackCounter++;
    if (UNLIKELY(m_ctx->m_stackCounter == 2048))
        throwEsprimaException(u"Maximum call stack size exceeded.", ctx, ErrorCode::RangeError);
}

ALWAYS_INLINE escargot::Node* isolateCoverGrammar(ParseContext* ctx, escargot::Node* (*parser) (ParseContext* ctx))
{
    PARSER_STACK_CHECK;

    bool oldIsBindingElement = ctx->m_isBindingElement,
        oldIsAssignmentTarget = ctx->m_isAssignmentTarget;
    RefPtr<ParseStatus> oldFirstCoverInitializedNameError = ctx->m_firstCoverInitializedNameError;
    escargot::Node* result;
    ctx->m_isBindingElement = true;
    ctx->m_isAssignmentTarget = true;
    ctx->m_firstCoverInitializedNameError = NULL;
    result = parser(ctx);
    if (ctx->m_firstCoverInitializedNameError) {
        throwEsprimaException();
    }
    ctx->m_isBindingElement = oldIsBindingElement;
    ctx->m_isAssignmentTarget = oldIsAssignmentTarget;
    ctx->m_firstCoverInitializedNameError = oldFirstCoverInitializedNameError;
    return result;
}

ALWAYS_INLINE escargot::Node* inheritCoverGrammar(ParseContext* ctx, escargot::Node* (*parser) (ParseContext* ctx))
{
    PARSER_STACK_CHECK;

    bool oldIsBindingElement = ctx->m_isBindingElement,
        oldIsAssignmentTarget = ctx->m_isAssignmentTarget;
    RefPtr<ParseStatus> oldFirstCoverInitializedNameError = ctx->m_firstCoverInitializedNameError;
    escargot::Node* result;
    ctx->m_isBindingElement = true;
    ctx->m_isAssignmentTarget = true;
    ctx->m_firstCoverInitializedNameError = NULL;
    result = parser(ctx);
    ctx->m_isBindingElement = ctx->m_isBindingElement && oldIsBindingElement;
    ctx->m_isAssignmentTarget = ctx->m_isAssignmentTarget && oldIsAssignmentTarget;
    // ctx->m_firstCoverInitializedNameError = oldFirstCoverInitializedNameError || ctx->m_firstCoverInitializedNameError;
    if (oldFirstCoverInitializedNameError)
        ctx->m_firstCoverInitializedNameError = oldFirstCoverInitializedNameError;
    return result;
}


ALWAYS_INLINE bool isIdentifierName(ParseStatus* token)
{
    return token->m_type == Token::IdentifierToken
        || token->m_type == Token::KeywordToken
        || token->m_type == Token::BooleanLiteralToken
        || token->m_type == Token::NullLiteralToken;
}

escargot::Node* finishLiteralNode(ParseContext* ctx, RefPtr<ParseStatus> ps)
{
    escargot::LiteralNode* nd;
    if (ps->m_type == Token::StringLiteralToken) {
        nd = new escargot::LiteralNode(ps->m_value.toESString());
    } else if (ps->m_type == Token::NumericLiteralToken) {
        nd = new escargot::LiteralNode(escargot::ESValue(ps->m_valueNumber));
    } else {
        RELEASE_ASSERT_NOT_REACHED();
    }
    nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    return nd;
}


void rearrangeNode(escargot::StatementNodeVector& body)
{
    /*
#ifndef NDEBUG
    puts("----------");
    for (size_t i = 0; i < body.size() ; i ++) {
        if (body[i]->type() == escargot::NodeType::FunctionDeclaration) {
            puts("FD");
        } else if (body[i]->type() == escargot::NodeType::VariableDeclarator) {
            puts("VariableDeclarator");
        } else {
            puts("O");
        }
    }
    puts("----------");
#endif
     */

    size_t firstNormalStatementPlace = SIZE_MAX; // not var, function decl.
    for (size_t i = 0; i < body.size() ; i ++) {
        if (body[i]->type() != escargot::NodeType::FunctionDeclaration
            && body[i]->type() != escargot::NodeType::VariableDeclarator) {
            firstNormalStatementPlace = i;
            break;
        }
    }
    if (firstNormalStatementPlace != SIZE_MAX) {
        for (size_t i = 0; i < firstNormalStatementPlace ; i ++) {
            if (body[i]->type() == escargot::NodeType::FunctionDeclaration) {
                for (size_t j = firstNormalStatementPlace - 1 ; j > i ; j --) {
                    if (body[j]->type() != escargot::NodeType::FunctionDeclaration) {
                        std::iter_swap(body.begin() + i, body.begin() + j);
                        break;
                    }
                }
            }
        }
    }

/*
#ifndef NDEBUG
    puts("----------");
    for (size_t i = 0; i < body.size() ; i ++) {
        if (body[i]->type() == escargot::NodeType::FunctionDeclaration) {
            puts("FD");
        } else if (body[i]->type() == escargot::NodeType::VariableDeclarator) {
            puts("VariableDeclaration");
        } else {
            puts("O");
        }
    }
    puts("----------");
#endif
 */

#ifndef NDEBUG
    if (firstNormalStatementPlace != SIZE_MAX) {
        bool findFD = false;
        for (size_t i = 0; i < body.size() ; i ++) {
            if (body[i]->type() == escargot::NodeType::FunctionDeclaration) {
                findFD = true;
            } else if (findFD && (body[i]->type() == escargot::NodeType::VariableDeclarator)) {
                ASSERT_NOT_REACHED();
            }
        }
    }
#endif
}

void reinterpretExpressionAsPattern(ParseContext* ctx, escargot::Node* expr);
escargot::Node* parseAssignmentExpression(ParseContext* ctx);
escargot::Node* parseFunctionDeclaration(ParseContext* ctx/*node, identifierIsOptional*/);
escargot::Node* parseYieldExpression(ParseContext* ctx);
escargot::Node* parseStatement(ParseContext* ctx);
escargot::Node* parsePattern(ParseContext* ctx, std::vector<RefPtr<ParseStatus> >& params);
escargot::Node* parseLeftHandSideExpression(ParseContext* ctx);
escargot::Node* parseNonComputedProperty(ParseContext* ctx);
escargot::Node* parsePatternWithDefault(ParseContext* ctx, std::vector<RefPtr<ParseStatus> >& params);
escargot::Node* parseExpression(ParseContext* ctx)
{
    escargot::Node* expr;
    // ParseStatus* startToken = ctx->m_lookahead;
    escargot::ExpressionNodeVector expressions;

    expr = isolateCoverGrammar(ctx, parseAssignmentExpression);

    if (match(ctx, Comma)) {
        expressions.clear();
        expressions.push_back(expr);

        while (ctx->m_startIndex < ctx->m_length) {
            if (!match(ctx, Comma)) {
                break;
            }
            lex(ctx);
            expressions.push_back(isolateCoverGrammar(ctx, parseAssignmentExpression));
        }

        // expr = new WrappingNode(startToken).finishSequenceExpression(expressions);
        expr = new escargot::SequenceExpressionNode(std::move(expressions));
        expr->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    }

    return expr;
}

// ECMA-262 13.2 Block

escargot::Node* parseStatementListItem(ParseContext* ctx)
{
    if (ctx->m_lookahead->m_type == Token::KeywordToken) {
        if (ctx->m_lookahead->m_keywordKind == Function)
            return parseFunctionDeclaration(ctx);
        /*
        switch (lookahead.value) {
        case 'export':
            if (sourceType !== 'module') {
                tolerateUnexpectedToken(lookahead, Messages.IllegalExportDeclaration);
            }
            return parseExportDeclaration();
        case 'import':
            if (sourceType !== 'module') {
                tolerateUnexpectedToken(lookahead, Messages.IllegalImportDeclaration);
            }
            return parseImportDeclaration();
        case 'const':
        case 'let':
            return parseLexicalDeclaration({inFor: false});
        case 'function':
            return parseFunctionDeclaration(new Node());
        case 'class':
            return parseClassDeclaration();
        }
         */
    }

    return parseStatement(ctx);
}

escargot::StatementNodeVector parseStatementList(ParseContext* ctx)
{
    escargot::StatementNodeVector list;
    // var list = [];
    while (ctx->m_startIndex < ctx->m_length) {
        if (match(ctx, RightBrace)) {
            break;
        }
        escargot::Node* nd = parseStatementListItem(ctx);
        if (nd)
            list.push_back(nd);
    }

    return std::move(list);
}

escargot::Node* parseBlock(ParseContext* ctx)
{
    // var block, node = new Node();
    escargot::StatementNodeVector body;
    expect(ctx, LeftBrace);

    body = parseStatementList(ctx);

    expect(ctx, RightBrace);
    escargot::Node* nd;
    nd = new escargot::BlockStatementNode(std::move(body));
    nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    return nd;
}

// ECMA-262 13.3.2 Variable Statement

escargot::Node* parseVariableIdentifier(ParseContext* ctx)
{
    // var token, node = new Node();
    RefPtr<ParseStatus> token;

    token = lex(ctx);

    if (token->m_type == Token::KeywordToken && token->m_keywordKind == Yield) {
        if (ctx->m_strict) {
            // tolerateUnexpectedToken(token, Messages.StrictReservedWord);
            tolerateUnexpectedToken();
        } if (!ctx->m_allowYield) {
            // throwUnexpectedToken(token);
            throwEsprimaException();
        }
    } else if (token->m_type != Token::IdentifierToken) {
        if (ctx->m_strict && token->m_type == Token::KeywordToken && token->m_keywordKind > StrictModeReservedWord) {
            tolerateUnexpectedToken();
            // tolerateUnexpectedToken(token, Messages.StrictReservedWord);
        } else {
            // throwUnexpectedToken(token);
            escargot::UTF16String err_msg;
            err_msg.append(u"Cannot use the keyword '");
            err_msg.append(tokenToString(token));
            if (ctx->m_isFunctionIdentifier) {
                err_msg.append(u"' as a function name.");
            } else {
                if (ctx->m_inCatch) {
                    err_msg.append(u"' as a catch variable name.");
                } else {
                    err_msg.append(u"' as a variable name.");
                }
            }
            throwEsprimaException(err_msg);
        }
    }
    /*
    else if (sourceType === 'module' && token.type === Token.Identifier && token.value === 'await') {
        tolerateUnexpectedToken(token);
    }
    */

    auto ll = token;
    escargot::Node* nd = new escargot::IdentifierNode(ll->m_value.toInternalAtomicString());
    nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    return nd;
}

escargot::VariableDeclaratorNode* parseVariableDeclaration(ParseContext* ctx)
{
    // var init = null, id, node = new Node(), params = [];
    escargot::Node* init = nullptr;
    std::vector<RefPtr<ParseStatus> > params;
    escargot::Node* id = parsePattern(ctx, params);

    // ECMA-262 12.2.1
    if (id->type() == escargot::NodeType::ObjectExpression)
        throw u"ES2015 Destructuring assignment syntax is not supported";
    // TODO: not alawys idenifier node!
    ASSERT(id->type() == escargot::NodeType::Identifier);
    if (ctx->m_strict && isRestrictedWord(((escargot::IdentifierNode *)id)->name())) {
        // tolerateError(Messages.StrictVarName);
        throwRestrictedWordUsedError(((escargot::IdentifierNode *)id)->name());
    }

    if (match(ctx, Substitution)) {
        lex(ctx);
        init = isolateCoverGrammar(ctx, parseAssignmentExpression);
    } else if (id->type() != escargot::NodeType::Identifier) {
        expect(ctx, Substitution);
    }

    // return node.finishVariableDeclarator(id, init);
    escargot::VariableDeclaratorNode* nd = new escargot::VariableDeclaratorNode(id, (escargot::ExpressionNode *)init);
    nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    return nd;
}

escargot::VariableDeclaratorVector parseVariableDeclarationList(ParseContext* ctx, bool excludeVariableDeclaratorNode = true)
{
    escargot::VariableDeclaratorVector list;
    // var list = [];

    do {
        escargot::VariableDeclaratorNode* node = parseVariableDeclaration(ctx);
        // ctx->m_currentBody->insert(ctx->m_currentBody->begin(), node);
        addDeclToCurrentContext(ctx, node);
        if (!excludeVariableDeclaratorNode)
            list.push_back(node);
        if (node->init()) {
            ASSERT(node->id()->type() == escargot::NodeType::Identifier);
            escargot::Node* id = ((escargot::IdentifierNode *) node->id())->clone();
            escargot::Node* init = node->init();
            node->clearInit();
            escargot::Node* nd = new escargot::AssignmentExpressionSimpleNode(id, init);
            nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
            list.push_back(nd);
        }


        if (!match(ctx, Comma)) {
            break;
        }
        lex(ctx);
    } while (ctx->m_startIndex < ctx->m_length);

    return list;
}

escargot::Node* parseVariableStatement(ParseContext* ctx /*node*/)
{
    escargot::VariableDeclaratorVector declarations;

    expectKeyword(ctx, Var);

    declarations = parseVariableDeclarationList(ctx);

    consumeSemicolon(ctx);

    // return node.finishVariableDeclaration(declarations);
    escargot::Node* nd = new escargot::VariableDeclarationNode(std::move(declarations));
    nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    return nd;
}

// ECMA-262 13.3.1 Let and Const Declarations

escargot::Node* parseLexicalBinding(ParseContext* ctx, escargot::UTF16String& kind /*options*/)
{
    // var init = null, id, node = new Node(), params = [];

    std::vector<RefPtr<ParseStatus> > params;
    escargot::Node* id = parsePattern(ctx, params);
    escargot::Node* init = nullptr;

    // ECMA-262 12.2.1
    if (ctx->m_strict && isRestrictedWord(((escargot::IdentifierNode *)id)->name())) {
        // tolerateError(Messages.StrictVarName);
        throwRestrictedWordUsedError(((escargot::IdentifierNode *)id)->name());
    }

    if (kind == u"const") {
        if (!matchKeyword(ctx, In) && !matchContextualKeyword(ctx, u"of")) {
            expect(ctx, Substitution);
            init = isolateCoverGrammar(ctx, parseAssignmentExpression);
        }
    }
    // FIXME options.inFor is not always true!
    if (match(ctx, Substitution)) {
        expect(ctx, Substitution);
        init = isolateCoverGrammar(ctx, parseAssignmentExpression);
    }
    /*
    else if ((!options.inFor && id.type !== Syntax.Identifier) || match('=')) {
        expect('=');
        init = isolateCoverGrammar(parseAssignmentExpression);
    }
     */

    // return node.finishVariableDeclarator(id, init);
    ASSERT(!init);
    escargot::Node* nd = new escargot::VariableDeclaratorNode(id);
    nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    return nd;
}
/*
function parseBindingList(kind, options) {
    var list = [];

    do {
        list.push(parseLexicalBinding(kind, options));
        if (!match(',')) {
            break;
        }
        lex();
    } while (startIndex < length);

    return list;
}
 */

escargot::Node* parseLexicalDeclaration(/*options*/)
{
    RELEASE_ASSERT_NOT_REACHED();
    /*
    var kind, declarations, node = new Node();

    kind = lex().value;
    assert(kind === 'let' || kind === 'const', 'Lexical declaration must be either let or const');

    declarations = parseBindingList(kind, options);

    consumeSemicolon();

    return node.finishLexicalDeclaration(declarations, kind);
     */
}

escargot::Node* parseRestElement(/*params*/)
{
    RELEASE_ASSERT_NOT_REACHED();
    /*
    var param, node = new Node();

    lex();

    if (match('{')) {
        throwError(Messages.ObjectPatternAsRestParameter);
    }

    params.push(lookahead);

    param = parseVariableIdentifier();

    if (match('=')) {
        throwError(Messages.DefaultRestParameter);
    }

    if (!match(')')) {
        throwError(Messages.ParameterAfterRestParameter);
    }

    return node.finishRestElement(param);
     */
}

// ECMA-262 13.4 Empty Statement

escargot::Node* parseEmptyStatement(ParseContext* ctx/*node*/)
{
    expect(ctx, SemiColon);
    // return node.finishEmptyStatement();
    escargot::Node* nd = new escargot::EmptyStatementNode();
    nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    return nd;
}

// ECMA-262 12.4 Expression Statement

escargot::Node* parseExpressionStatement(ParseContext* ctx/*node*/)
{
    escargot::Node* expr = parseExpression(ctx);
    consumeSemicolon(ctx);
    // return node.finishExpressionStatement(expr);
    escargot::Node* nd = new escargot::ExpressionStatementNode(expr);
    nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    return nd;
}

// ECMA-262 13.6 If statement

escargot::Node* parseIfStatement(ParseContext* ctx/*node*/)
{
    escargot::Node* test;
    escargot::Node* consequent;
    escargot::Node* alternate;

    expectKeyword(ctx, If);

    expect(ctx, LeftParenthesis);

    test = parseExpression(ctx);

    expect(ctx, RightParenthesis);

    consequent = parseStatement(ctx);

    if (matchKeyword(ctx, Else)) {
        lex(ctx);
        alternate = parseStatement(ctx);
    } else {
        alternate = nullptr;
    }

    // return node.finishIfStatement(test, consequent, alternate);
    escargot::Node* nd = new escargot::IfStatementNode(test, consequent, alternate);
    nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    return nd;
}

// ECMA-262 13.7 Iteration Statements

escargot::Node* parseDoWhileStatement(ParseContext* ctx/*node*/)
{
    bool oldInIteration;
    escargot::Node* body;
    escargot::Node* test;

    expectKeyword(ctx, Do);

    oldInIteration = ctx->m_inIteration;
    ctx->m_inIteration = true;

    body = parseStatement(ctx);

    ctx->m_inIteration = oldInIteration;

    expectKeyword(ctx, While);

    expect(ctx, LeftParenthesis);

    test = parseExpression(ctx);

    expect(ctx, RightParenthesis);

    if (match(ctx, SemiColon)) {
        lex(ctx);
    }

    // return node.finishDoWhileStatement(body, test);
    escargot::Node* nd = new escargot::DoWhileStatementNode(test, body);
    nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    return nd;
}

escargot::Node* parseWhileStatement(ParseContext* ctx/*node*/)
{
    bool oldInIteration;
    escargot::Node* body;
    escargot::Node* test;

    expectKeyword(ctx, While);

    expect(ctx, LeftParenthesis);

    test = parseExpression(ctx);

    expect(ctx, RightParenthesis);

    oldInIteration = ctx->m_inIteration;
    ctx->m_inIteration = true;

    body = parseStatement(ctx);

    ctx->m_inIteration = oldInIteration;

    // return node.finishWhileStatement(test, body);
    escargot::Node* nd = new escargot::WhileStatementNode(test, body);
    nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    return nd;
}

escargot::Node* parseForStatement(ParseContext* ctx/*node*/)
{
    // var init, forIn, initSeq, initStartToken, test, update, left, right, kind, declarations,
    // body, oldInIteration, previousAllowIn = state.allowIn;
    escargot::Node* init;
    escargot::Node* test;
    escargot::Node* update;
    escargot::Node* left = NULL;
    escargot::Node* right = NULL;
    escargot::Node* body;
    bool previousAllowIn = ctx->m_allowIn;

    init = test = update = nullptr;
    bool forIn = true;

    expectKeyword(ctx, For);

    expect(ctx, LeftParenthesis);

    if (match(ctx, SemiColon)) {
        lex(ctx);
    } else {
        if (matchKeyword(ctx, Var)) {
            // init = new Node();
            lex(ctx);

            ctx->m_allowIn = false;
            init = new escargot::VariableDeclarationNode(parseVariableDeclarationList(ctx, false));
            init->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
            ctx->m_allowIn = previousAllowIn;

            if (((escargot::VariableDeclarationNode *)init)->declarations().size() == 1 && matchKeyword(ctx, In)) {
                lex(ctx);
                // left = init;
                left = ((escargot::VariableDeclaratorNode *)(((escargot::VariableDeclarationNode *)init)->declarations()[0]))->id();
                right = parseExpression(ctx);
                init = nullptr;
            } else if (((escargot::VariableDeclarationNode *)init)->declarations().size() == 1 && ((escargot::VariableDeclaratorNode *)((escargot::VariableDeclarationNode *)init)->declarations()[0])->init() == nullptr
                && matchContextualKeyword(ctx, u"of")) {
                lex(ctx);
                left = init;
                right = parseAssignmentExpression(ctx);
                init = nullptr;
                forIn = false;
            } else {
                escargot::VariableDeclaratorVector& vec = ((escargot::VariableDeclarationNode *)init)->declarations();
                for (unsigned i = 0 ; i < vec.size() ; i ++) {
                    if (vec[i]->type() == escargot::NodeType::VariableDeclarator) {
                        vec.erase(vec.begin() + i);
                        i = 0;
                    }
                }
                expect(ctx, SemiColon);
            }
        } else {
            // ParseStatus* initStartToken = ctx->m_lookahead;
            ctx->m_allowIn = false;
            init = inheritCoverGrammar(ctx, parseAssignmentExpression);
            ctx->m_allowIn = previousAllowIn;

            if (matchKeyword(ctx, In)) {
                if (!ctx->m_isAssignmentTarget) {
                    // tolerateError(u"Messages.InvalidLHSInForIn");
                }

                lex(ctx);
                reinterpretExpressionAsPattern(ctx, init);
                left = init;
                right = parseExpression(ctx);
                init = nullptr;
            } else if (matchContextualKeyword(ctx, u"of")) {
                if (!ctx->m_isAssignmentTarget) {
                    // tolerateError(u"Messages.InvalidLHSInForLoop");
                }

                lex(ctx);
                reinterpretExpressionAsPattern(ctx, init);
                left = init;
                right = parseAssignmentExpression(ctx);
                init = nullptr;
                forIn = false;
            } else {
                escargot::ExpressionNodeVector initSeq;
                if (match(ctx, Comma)) {
                    initSeq.push_back(init);
                    // initSeq = [init];
                    while (match(ctx, Comma)) {
                        lex(ctx);
                        // initSeq.push(isolateCoverGrammar(parseAssignmentExpression));
                        initSeq.push_back(isolateCoverGrammar(ctx, parseAssignmentExpression));
                    }
                    // init = new WrappingNode(initStartToken).finishSequenceExpression(initSeq);
                    init = new escargot::SequenceExpressionNode(std::move(initSeq));
                    init->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
                }
                expect(ctx, SemiColon);
            }
        }
    }

    // if (typeof left === 'undefined') {
    if (left == NULL) {

        if (!match(ctx, SemiColon)) {
            test = parseExpression(ctx);
        }
        expect(ctx, SemiColon);

        if (!match(ctx, RightParenthesis)) {
            update = parseExpression(ctx);
        }
    }

    expect(ctx, RightParenthesis);

    bool oldInIteration = ctx->m_inIteration;
    ctx->m_inIteration = true;

    body = isolateCoverGrammar(ctx, parseStatement);

    ctx->m_inIteration = oldInIteration;

    if (left == NULL) {
        escargot::Node* nd = new escargot::ForStatementNode(init, test, update, body);
        nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
        return nd;
    } else {
        if (forIn) {
            // FIXME what is each?
            escargot::Node* nd = new escargot::ForInStatementNode(left, right, body, false);
            nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
            return nd;
        } else {
            throw u"ES2015 for..of statement is not supported";
        }
    }
    /*
    return (typeof left === 'undefined') ?
            node.finishForStatement(init, test, update, body) :
            forIn ? node.finishForInStatement(left, right, body) :
                    node.finishForOfStatement(left, right, body);
     */

}

// ECMA-262 13.8 The continue statement

escargot::Node* parseContinueStatement(ParseContext* ctx/*node*/)
{
    // var label = null, key;

    expectKeyword(ctx, Continue);

    // Optimize the most common form: 'continue;'.
    if (ctx->m_sourceString->charAt(ctx->m_startIndex) == 0x3B) {
        lex(ctx);

        if (!ctx->m_inIteration) {
            // throwError(Messages.IllegalContinue);
            throw u"Illegal continue statement";
        }

        // return node.finishContinueStatement(null);
        escargot::Node* nd = new escargot::ContinueStatementNode();
        nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
        return nd;
    }

    if (ctx->m_hasLineTerminator) {
        if (!ctx->m_inIteration) {
            // throwError(Messages.IllegalContinue);
            throw u"Illegal continue statement";
        }

        // return node.finishContinueStatement(null);
        escargot::Node* nd = new escargot::ContinueStatementNode();
        nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
        return nd;
    }

    escargot::Node* label = NULL;
    size_t upCount = 0;
    if (ctx->m_lookahead->m_type == Token::IdentifierToken) {
        label = parseVariableIdentifier(ctx);
        escargot::ESString* key = ((escargot::IdentifierNode *)label)->nonAtomicName();

        auto iter = ctx->m_labelSet.rbegin();
        bool find = false;
        while (iter != ctx->m_labelSet.rend()) {
            if (*(iter->first) == *key) {
                if (!iter->second)
                    throw u"Cannot continue to the label as it is not targeting a loop.";
                find = true;
                break;
            }
            upCount++;
            iter++;
        }
        if (!find) {
            throw u"Error(Messages.UnknownLabel, label.name)";
        }
    }

    consumeSemicolon(ctx);

    if (label == NULL && !(ctx->m_inIteration || ctx->m_inSwitch)) {
        // throwError(Messages.IllegalContinue);
        throw u"Illegal continue statement";
    }

    if (label) {
        escargot::Node* nd = new escargot::ContinueLabelStatementNode(upCount, ((escargot::IdentifierNode *)label)->nonAtomicName());
        nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
        return nd;
    } else {
        escargot::Node* nd = new escargot::ContinueStatementNode();
        nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
        return nd;
    }
}

// ECMA-262 13.9 The break statement

escargot::Node* parseBreakStatement(ParseContext* ctx/*node*/)
{
    // var label = null, key;

    expectKeyword(ctx, Break);

    // Catch the very common case first: immediately a semicolon (U+003B).
    if (ctx->m_sourceString->charAt(ctx->m_lastIndex) == 0x3B) {
        lex(ctx);

        if (!(ctx->m_inIteration || ctx->m_inSwitch)) {
            // throwError(Messages.IllegalBreak);
            throw u"Messages.IllegalBreak";
        }

        // return node.finishBreakStatement(null);
        escargot::Node* nd = new escargot::BreakStatementNode();
        nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
        return nd;
    }

    if (ctx->m_hasLineTerminator) {
        if (!(ctx->m_inIteration || ctx->m_inSwitch)) {
            // throwError(Messages.IllegalBreak);
            throw u"Messages.IllegalBreak";
        }
        escargot::Node* nd = new escargot::BreakStatementNode();
        nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
        return nd;
    }

    escargot::Node* label = NULL;
    size_t upCount = 0;
    if (ctx->m_lookahead->m_type == Token::IdentifierToken) {
        label = parseVariableIdentifier(ctx);
        escargot::ESString* key = ((escargot::IdentifierNode *)label)->nonAtomicName();

        auto iter = ctx->m_labelSet.rbegin();
        bool find = false;
        while (iter != ctx->m_labelSet.rend()) {
            if (*(iter->first) == *key) {
                find = true;
                break;
            }
            upCount++;
            iter++;
        }
        if (!find) {
            throw u"Error(Messages.UnknownLabel, label.name)";
        }
    }

    consumeSemicolon(ctx);

    if (label == NULL && !(ctx->m_inIteration || ctx->m_inSwitch)) {
        throw u"throwError(Messages.IllegalBreak);";
    }
    if (label) {
        escargot::Node* nd = new escargot::BreakLabelStatementNode(upCount, ((escargot::IdentifierNode *)label)->nonAtomicName());
        nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
        return nd;
    } else {
        escargot::Node* nd = new escargot::BreakStatementNode();
        nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
        return nd;
    }
}

// ECMA-262 13.10 The return statement

escargot::Node* parseReturnStatement(ParseContext* ctx/*node*/)
{
    escargot::Node* argument = nullptr;
    // var argument = null;

    expectKeyword(ctx, Return);

    if (!ctx->m_inFunctionBody) {
        // tolerateError(Messages.IllegalReturn);
        throw u"Messages.IllegalReturn";
    }

    // 'return' followed by a space and an identifier is very common.
    if (ctx->m_sourceString->charAt(ctx->m_lastIndex) == 0x20) {
        if (isIdentifierStart(ctx->m_sourceString->charAt(ctx->m_lastIndex))) {
            argument = parseExpression(ctx);
            consumeSemicolon(ctx);
            escargot::Node* nd = new escargot::ReturnStatmentNode(argument);
            nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
            return nd;
        }
    }

    if (ctx->m_hasLineTerminator) {
        // HACK
        // return node.finishReturnStatement(null);
        escargot::Node* nd = new escargot::ReturnStatmentNode(nullptr);
        nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
        return nd;
    }

    if (!match(ctx, SemiColon)) {
        if (!match(ctx, RightBrace) && ctx->m_lookahead->m_type != Token::EOFToken) {
            argument = parseExpression(ctx);
        }
    }

    consumeSemicolon(ctx);

    escargot::Node* nd = new escargot::ReturnStatmentNode(argument);
    nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    return nd;
}

// ECMA-262 13.11 The with statement

escargot::Node* parseWithStatement(ParseContext* ctx)
{
    throw u"with statement is not supported";
}

// ECMA-262 13.12 The switch statement

escargot::SwitchCaseNode* parseSwitchCase(ParseContext* ctx)
{
    // var test, consequent = [], statement, node = new Node();
    escargot::Node* test; // , consequent = [], statement, node = new Node();
    escargot::StatementNodeVector consequent;
    if (matchKeyword(ctx, Default)) {
        lex(ctx);
        test = nullptr;
    } else {
        expectKeyword(ctx, Case);
        test = parseExpression(ctx);
    }
    expect(ctx, Colon);

    while (ctx->m_startIndex < ctx->m_length) {
        if (match(ctx, RightBrace) || matchKeyword(ctx, Default) || matchKeyword(ctx, Case)) {
            break;
        }
        escargot::Node* statement = parseStatementListItem(ctx);
        if (statement)
            consequent.push_back(statement);
    }

    // return node.finishSwitchCase(test, consequent);
    escargot::SwitchCaseNode* nd = new escargot::SwitchCaseNode(test, std::move(consequent));
    nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    return nd;
}

escargot::Node* parseSwitchStatement(ParseContext* ctx/*node*/)
{
    // var discriminant, cases, clause, oldInSwitch, defaultFound;

    expectKeyword(ctx, Switch);

    expect(ctx, LeftParenthesis);

    escargot::Node* discriminant = parseExpression(ctx);

    expect(ctx, RightParenthesis);

    expect(ctx, LeftBrace);

    escargot::StatementNodeVector casesA;
    escargot::StatementNodeVector casesB;
    // cases = [];

    if (match(ctx, RightBrace)) {
        lex(ctx);
        // return node.finishSwitchStatement(discriminant, cases);
        escargot::Node* nd = new escargot::SwitchStatementNode(discriminant, std::move(casesA), nullptr, std::move(casesB), false);
        nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
        return nd;
    }

    bool oldInSwitch = ctx->m_inSwitch;
    ctx->m_inSwitch = true;
    bool defaultFound = false;
    escargot::Node* def = nullptr;
    while (ctx->m_startIndex < ctx->m_length) {
        if (match(ctx, RightBrace)) {
            break;
        }
        escargot::SwitchCaseNode* clause = parseSwitchCase(ctx);

        if (clause->isDefaultNode()) {
            if (defaultFound) {
                throw u"Messages.MultipleDefaultsInSwitch";
            }
            defaultFound = true;
            def = clause;
        } else if (defaultFound)
            casesA.push_back(clause);
        else
            casesB.push_back(clause);
    }

    ctx->m_inSwitch = oldInSwitch;

    expect(ctx, RightBrace);

    // return node.finishSwitchStatement(discriminant, cases);
    escargot::Node* nd = new escargot::SwitchStatementNode(discriminant, std::move(casesA), def, std::move(casesB), false);
    nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    return nd;
}

// ECMA-262 13.14 The throw statement

escargot::Node* parseThrowStatement(ParseContext* ctx/*node*/)
{
    escargot::Node* argument;

    expectKeyword(ctx, Throw);

    if (ctx->m_hasLineTerminator) {
        throw u"Messages.NewlineAfterThrow";
        // throwError(Messages.NewlineAfterThrow);
    }

    argument = parseExpression(ctx);

    consumeSemicolon(ctx);

    // return node.finishThrowStatement(argument);
    escargot::Node* nd = new escargot::ThrowStatementNode(argument);
    nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    return nd;
}

// ECMA-262 13.15 The try statement

escargot::Node* parseCatchClause(ParseContext* ctx/*node*/)
{
    // var param, params = [], paramMap = {}, key, i, body, node = new Node();
    ctx->m_inCatch = true;
    expectKeyword(ctx, Catch);

    expect(ctx, LeftParenthesis);
    if (match(ctx, RightParenthesis)) {
        // throwUnexpectedToken(lookahead);
        throwEsprimaException();
    }

    std::vector<RefPtr<ParseStatus> > params;

    escargot::Node* param = parsePattern(ctx, params);
    for (unsigned i = 0; i < params.size(); i++) {
        // TODO
        /*
        key = '$' + params[i].value;
        if (Object.prototype.hasOwnProperty.call(paramMap, key)) {
            tolerateError(Messages.DuplicateBinding, params[i].value);
        }
        paramMap[key] = true;
         */
    }

    // ECMA-262 12.14.1
    if (ctx->m_strict && isRestrictedWord(((escargot::IdentifierNode*)param)->name())) {
        throwRestrictedWordUsedError(((escargot::IdentifierNode *)param)->name());
    }

    expect(ctx, RightParenthesis);
    escargot::Node* body = parseBlock(ctx);
    // return node.finishCatchClause(param, body);
    escargot::Node* nd = new escargot::CatchClauseNode(param, nullptr, body);
    nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    ctx->m_inCatch = false;
    return nd;
}

escargot::Node* parseTryStatement(ParseContext* ctx/*node*/)
{
    // var block, handler = null, finalizer = null;

    expectKeyword(ctx, Try);

    escargot::Node* block = parseBlock(ctx);
    escargot::Node* handler = nullptr;
    escargot::Node* finalizer = nullptr;

    if (matchKeyword(ctx, Catch)) {
        handler = parseCatchClause(ctx);
    }

    if (matchKeyword(ctx, Finally)) {
        lex(ctx);
        finalizer = parseBlock(ctx);
    }

    if (!handler && !finalizer) {
        throw u"Try statement do not have catch or finally clause";
    }

    // return node.finishTryStatement(block, handler, finalizer);
    escargot::Node* nd = new escargot::TryStatementNode(block, handler, escargot::CatchClauseNodeVector(), finalizer);
    nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    return nd;
}

// ECMA-262 13.16 The debugger statement
/*
function parseDebuggerStatement(node) {
    expectKeyword('debugger');

    consumeSemicolon();

    return node.finishDebuggerStatement();
}
*/
// 13 Statements

escargot::Node* parseStatement(ParseContext* ctx)
{
    PARSER_STACK_CHECK;

    Token type = ctx->m_lookahead->m_type;
    /*
    var type = lookahead.type,
                expr,
                labeledBody,
                key,
                node;
     */

    if (type == Token::EOFToken) {
        // throwUnexpectedToken(lookahead);
        throwEsprimaException();
    }

    if (type == Token::PunctuatorToken && ctx->m_lookahead->m_punctuatorsKind == LeftBrace) {
        return parseBlock(ctx);
    }
    ctx->m_isAssignmentTarget = ctx->m_isBindingElement = true;

    if (type == Token::PunctuatorToken) {
        if (ctx->m_lookahead->m_punctuatorsKind == SemiColon) {
            return parseEmptyStatement(ctx);
        } else if (ctx->m_lookahead->m_punctuatorsKind == LeftParenthesis) {
            return parseExpressionStatement(ctx);
        }
    } else if (type == Token::KeywordToken) {
        if (ctx->m_lookahead->m_keywordKind == Break) {
            return parseBreakStatement(ctx);
        } else if (ctx->m_lookahead->m_keywordKind == Continue) {
            return parseContinueStatement(ctx);
        } else if (ctx->m_lookahead->m_keywordKind == Debugger) {
            RELEASE_ASSERT_NOT_REACHED();
        } else if (ctx->m_lookahead->m_keywordKind == Do) {
            return parseDoWhileStatement(ctx);
        } else if (ctx->m_lookahead->m_keywordKind == For) {
            return parseForStatement(ctx);
        } else if (ctx->m_lookahead->m_keywordKind == Function) {
            throwEsprimaException(u"Function declaration cannot be used as statement");
        } else if (ctx->m_lookahead->m_keywordKind == If) {
            return parseIfStatement(ctx);
        } else if (ctx->m_lookahead->m_keywordKind == Return) {
            return parseReturnStatement(ctx);
        } else if (ctx->m_lookahead->m_keywordKind == Switch) {
            return parseSwitchStatement(ctx);
        } else if (ctx->m_lookahead->m_keywordKind == Throw) {
            return parseThrowStatement(ctx);
        } else if (ctx->m_lookahead->m_keywordKind == Try) {
            return parseTryStatement(ctx);
        } else if (ctx->m_lookahead->m_keywordKind == Var) {
            return parseVariableStatement(ctx);
        } else if (ctx->m_lookahead->m_keywordKind == While) {
            return parseWhileStatement(ctx);
        } else if (ctx->m_lookahead->m_keywordKind == With) {
            return parseWithStatement(ctx);
        }

    }

    escargot::Node* expr = parseExpression(ctx);

    // ECMA-262 12.12 Labelled Statements
    if ((expr->type() == escargot::NodeType::Identifier) && match(ctx, Colon)) {
        std::vector<escargot::ESString*> labels;
        do {
            lex(ctx); // consume semicolon

            escargot::ESString* key = ((escargot::IdentifierNode *)expr)->nonAtomicName();
            auto iter = ctx->m_labelSet.begin();
            while (iter != ctx->m_labelSet.end()) {
                if (*(iter->first) == *key) {
                    throw u"throwError(Messages.Redeclaration, 'Label', expr.name);";
                }
                iter++;
            }

            labels.push_back(key);

            if (ctx->m_lookahead->m_type == Token::IdentifierToken && ctx->m_sourceString->charAt(ctx->m_index) == ':') {
                expr = parseExpression(ctx);
                ASSERT(expr->isIdentifier());
            } else {
                break;
            }

        } while (true);

        bool labelTargetsLoop = ctx->m_lookahead->m_type == Token::KeywordToken
            && ((ctx->m_lookahead->m_keywordKind == KeywordKind::For)
            || (ctx->m_lookahead->m_keywordKind == KeywordKind::Do)
            || (ctx->m_lookahead->m_keywordKind == KeywordKind::While));

        for (auto label : labels) {
            ctx->m_labelSet.push_back(std::make_pair(label, labelTargetsLoop));
        }
        escargot::Node* labeledBody = parseStatement(ctx);
        escargot::LabeledStatementNode* nd = (escargot::LabeledStatementNode*)labeledBody;
        for (auto label : labels) {
            nd = new escargot::LabeledStatementNode((escargot::StatementNode*)nd, label);
            ctx->m_labelSet.pop_back();
        }

        nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
        return nd;
    }

    consumeSemicolon(ctx);

    // return node.finishExpressionStatement(expr);
    escargot::Node* nd = new escargot::ExpressionStatementNode(expr);
    nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    return nd;
}

// ECMA-262 14.1 Function Definition

escargot::Node* parseFunctionSourceElements(ParseContext* ctx)
{
    /*var statement, body = [], token, directive, firstRestricted,
        oldLabelSet, oldInIteration, oldInSwitch, oldInFunctionBody, oldParenthesisCount,
        node = new Node();
     */

    expect(ctx, LeftBrace);
    escargot::StatementNodeVector* prevBody = ctx->m_currentBody;
    escargot::StatementNodeVector body;
    ctx->m_currentBody = &body;
    RefPtr<ParseStatus> firstRestricted;

    while (ctx->m_startIndex < ctx->m_length) {
        if (ctx->m_lookahead->m_type != Token::StringLiteralToken) {
            break;
        }
        RefPtr<ParseStatus> token = ctx->m_lookahead;

        bool strict = true;
        if (token->m_end - 1 - (token->m_start + 1) == 10) {
            static const char16_t* s = u"use strict";
            for (size_t i = 0 ; i < 10 ; i ++) {
                if (s[i] != ctx->m_sourceString->charAt(token->m_start + 1 + i)) {
                    strict = false;
                }
            }
        } else {
            strict = false;
        }

        escargot::Node* statement = parseStatementListItem(ctx);
        if (statement)
            body.push_back(statement);
        else
            break;
        ASSERT(statement->type() == escargot::NodeType::ExpressionStatement);
        if (((escargot::ExpressionStatementNode *) statement)->expression()->type() != escargot::NodeType::Literal) {
            // this is not directive
            break;
        } else {
            if (strict) {
                ctx->m_strict = true;
                if (firstRestricted) {
                    // tolerateUnexpectedToken(firstRestricted, Messages.StrictOctalLiteral);
                    throwOctalLiteralUsedError();
                }
            } else {
                if (!firstRestricted && token->m_octal) {
                    firstRestricted = token;
                }
            }
        }

        // directive = source.slice(token.start + 1, token.end - 1);
        /*
        escargot::UTF16String directive = ctx->m_source.substr(token->m_start + 1,
            token->m_end - 1 - (token->m_start + 1));
        // directive = source.slice(token.start + 1, token.end - 1);
        if (directive == u"use strict") {
            ctx->m_strict = true;
            if (firstRestricted) {
                // tolerateUnexpectedToken(firstRestricted, Messages.StrictOctalLiteral);
                tolerateUnexpectedToken();
            }
        } else {
            if (!firstRestricted && token->m_octal) {
                firstRestricted = token;
            }
        }
        */
    }

    std::vector<std::pair<escargot::ESString *, bool> > oldLabelSet = ctx->m_labelSet;
    bool oldInIteration = ctx->m_inIteration;
    bool oldInSwitch = ctx->m_inSwitch;
    bool oldInFunctionBody = ctx->m_inFunctionBody;
    int oldParenthesisCount = ctx->m_parenthesizedCount;

    ctx->m_labelSet.clear();
    ctx->m_inIteration = false;
    ctx->m_inSwitch = false;
    ctx->m_inFunctionBody = true;
    ctx->m_parenthesizedCount = 0;

    while (ctx->m_startIndex < ctx->m_length) {
        if (match(ctx, RightBrace)) {
            break;
        }
        escargot::Node* nd = parseStatementListItem(ctx);
        if (nd)
            body.push_back(nd);
    }

    expect(ctx, RightBrace);

    ctx->m_labelSet = oldLabelSet;
    ctx->m_inIteration = oldInIteration;
    ctx->m_inSwitch = oldInSwitch;
    ctx->m_inFunctionBody = oldInFunctionBody;
    ctx->m_parenthesizedCount = oldParenthesisCount;

    rearrangeNode(*ctx->m_currentBody);

    ctx->m_currentBody = prevBody;
    escargot::Node* nd;
    nd = new escargot::BlockStatementNode(std::move(body));
    nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    return nd;
}


/*
function validateParam(options, param, name) {
    var key = '$' + name;
    if (strict) {
        if (isRestrictedWord(name)) {
            options.stricted = param;
            options.message = Messages.StrictParamName;
        }
        if (Object.prototype.hasOwnProperty.call(options.paramSet, key)) {
            options.stricted = param;
            options.message = Messages.StrictParamDupe;
        }
    } else if (!options.firstRestricted) {
        if (isRestrictedWord(name)) {
            options.firstRestricted = param;
            options.message = Messages.StrictParamName;
        } else if (isStrictModeReservedWord(name)) {
            options.firstRestricted = param;
            options.message = Messages.StrictReservedWord;
        } else if (Object.prototype.hasOwnProperty.call(options.paramSet, key)) {
            options.stricted = param;
            options.message = Messages.StrictParamDupe;
        }
    }
    options.paramSet[key] = true;
}
*/


/*
function parseParam(options) {
    var token, param, params = [], i, def;

    token = lookahead;
    if (token.value === '...') {
        param = parseRestElement(params);
        validateParam(options, param.argument, param.argument.name);
        options.params.push(param);
        options.defaults.push(null);
        return false;
    }

    param = parsePatternWithDefault(params);
    for (i = 0; i < params.length; i++) {
        validateParam(options, params[i], params[i].value);
    }

    if (param.type === Syntax.AssignmentPattern) {
        def = param.right;
        param = param.left;
        ++options.defaultCount;
    }

    options.params.push(param);
    options.defaults.push(def);

    return !match(')');
}
 */

bool parseParam(ParseContext* ctx, escargot::InternalAtomicStringVector& vec)
{
    // var token, param, params = [], i, def;

    RefPtr<ParseStatus> token = ctx->m_lookahead;
    /*
    if (token.value === '...') {
        param = parseRestElement(params);
        validateParam(options, param.argument, param.argument.name);
        options.params.push(param);
        options.defaults.push(null);
        return false;
    }
    */

    std::vector<RefPtr<ParseStatus> > params;
    escargot::Node* param = parsePatternWithDefault(ctx, params);

    if (ctx->m_strict) {
        if (isRestrictedWord(((escargot::IdentifierNode*)param)->name())) {
            throwRestrictedWordUsedError(((escargot::IdentifierNode *)param)->name());
        }
        if (std::find(vec.begin(), vec.end(), ((escargot::IdentifierNode*)param)->name()) != vec.end()) {
            throwDuplicateIdentifierError(((escargot::IdentifierNode*)param)->name());
        }
    }

    /*
    for (i = 0; i < params.length; i++) {
        validateParam(options, params[i], params[i].value);
    }

    if (param.type === Syntax.AssignmentPattern) {
        def = param.right;
        param = param.left;
        ++options.defaultCount;
    }
    */

    // options.params.push(param);
    // options.defaults.push(def);
    ASSERT(param->type() == escargot::NodeType::Identifier);
    vec.push_back(((escargot::IdentifierNode *)param)->name());
    return !match(ctx, RightParenthesis);
}

escargot::InternalAtomicStringVector parseParams(ParseContext* ctx)
{
    /*
    var options;

    options = {
        params: [],
        defaultCount: 0,
        defaults: [],
        firstRestricted: firstRestricted
    };
    */

    escargot::InternalAtomicStringVector vec;
    expect(ctx, LeftParenthesis);

    if (!match(ctx, RightParenthesis)) {
        while (ctx->m_startIndex < ctx->m_length) {
            if (!parseParam(ctx, vec)) {
                break;
            }
            expect(ctx, Comma);
        }
        /*
        options.paramSet = {};
        while (startIndex < length) {
            if (!parseParam(options)) {
                break;
        }
        expect(',');
        }
        */
    }

    expect(ctx, RightParenthesis);

    /*
    if (options.defaultCount === 0) {
        options.defaults = [];
    }

    return {
        params: options.params,
        defaults: options.defaults,
        stricted: options.stricted,
        firstRestricted: options.firstRestricted,
        message: options.message
    };
    */

    return vec;
}

template <typename Type>
void validateFunctionParamsLazily(ParseContext* ctx, Type* nd, RefPtr<ParseStatus>& firstRestricted, bool previousStrict)
{
    if (!previousStrict && ctx->m_strict) {
        if (firstRestricted) {
            throwRestrictedWordUsedError(firstRestricted);
        } else {
            Type* f = static_cast<Type *>(nd);
            const escargot::InternalAtomicStringVector& params = f->params();
            escargot::InternalAtomicStringVector::const_iterator it = params.begin();
            for (escargot::InternalAtomicString param : params) {

                if (isRestrictedWord(param)) {
                    throwRestrictedWordUsedError(param);
                }

                if (isStrictModeReservedWord(param)) {
                    throwRestrictedWordUsedError(param);
                }

                it++;
                if (std::find(it, params.end(), param) != params.end()) {
                    throwDuplicateIdentifierError(param);
                }
            }
        }
    }
}

escargot::Node* parseFunctionDeclaration(ParseContext* ctx/*node, identifierIsOptional*/)
{
    // var id = null, params = [], defaults = [], body, token, stricted, tmp, firstRestricted, message, previousStrict,
    // isGenerator, previousAllowYield;

    bool previousAllowYield = ctx->m_allowYield;
    RefPtr<ParseStatus> firstRestricted;
    expectKeyword(ctx, Function);

    bool isGenerator = match(ctx, Multiply);
    if (isGenerator) {
        lex(ctx);
    }

    // if (!identifierIsOptional || !match('(')) {
    escargot::Node* id;
    if (!match(ctx, LeftParenthesis)) {
        RefPtr<ParseStatus> token = ctx->m_lookahead;
        ctx->m_isFunctionIdentifier = true;
        id = parseVariableIdentifier(ctx);
        ctx->m_isFunctionIdentifier = false;
        if (ctx->m_strict) {
            if (isRestrictedWord(token->m_value)) {
                // tolerateUnexpectedToken(token, Messages.StrictFunctionName);
                throwRestrictedWordUsedError(token->m_value.toInternalAtomicString());
            }
        } else {
            if (!firstRestricted) {
                if (isRestrictedWord(token->m_value)) {
                    firstRestricted = token;
                    // message = Messages.StrictFunctionName;
                } else if (isStrictModeReservedWord(token->m_value)) {
                    firstRestricted = token;
                    // message = Messages.StrictReservedWord;
                }
            }
        }
    } else {
        throwEsprimaException(u"Unnamed function declaration is forbidden");
        RELEASE_ASSERT_NOT_REACHED();
    }

    ctx->m_allowYield = !isGenerator;
    escargot::InternalAtomicStringVector params = parseParams(ctx);
    /*
    tmp = parseParams(firstRestricted);
    params = tmp.params;
    defaults = tmp.defaults;
    stricted = tmp.stricted;
    firstRestricted = tmp.firstRestricted;
    if (tmp.message) {
        message = tmp.message;
    }
    */


    bool previousStrict = ctx->m_strict;
    escargot::Node* body = parseFunctionSourceElements(ctx);

    // return node.finishFunctionDeclaration(id, params, defaults, body, isGenerator);
    ASSERT(id->type() == escargot::NodeType::Identifier);

    escargot::FunctionDeclarationNode* nd = new escargot::FunctionDeclarationNode(((escargot::IdentifierNode *)id)->name(), std::move(params), body, isGenerator, false, ctx->m_strict);
    nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    ctx->m_currentBody->insert(ctx->m_currentBody->begin(), nd);
    validateFunctionParamsLazily(ctx, nd, firstRestricted, previousStrict);

    escargot::IdentifierNode* idNode = new escargot::IdentifierNode(((escargot::IdentifierNode *)id)->name());
    idNode->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    escargot::VariableDeclaratorNode* v = new escargot::VariableDeclaratorNode(
        idNode,
        nullptr,
        true
    );

    ctx->m_currentBody->insert(ctx->m_currentBody->begin(), v);
    ctx->m_strict = previousStrict;
    ctx->m_allowYield = previousAllowYield;

    return NULL;
}

escargot::StatementNodeVector makeAnonymousFunctionNodeVector(ParseContext* ctx, escargot::InternalAtomicStringVector& vec, escargot::Node* body)
{
    escargot::StatementNodeVector stmtNodeVec;
    RefPtr<ParseStatus> firstRestricted;

    escargot::FunctionDeclarationNode* nd = new escargot::FunctionDeclarationNode(escargot::InternalAtomicString("anonymous"), std::move(vec), body, false, false, ctx->m_strict);
    nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    stmtNodeVec.insert(stmtNodeVec.begin(), nd);
    validateFunctionParamsLazily(ctx, nd, firstRestricted, false);

    escargot::IdentifierNode* idNode = new escargot::IdentifierNode(escargot::InternalAtomicString("anonymous"));
    idNode->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    escargot::VariableDeclaratorNode* v = new escargot::VariableDeclaratorNode(
        idNode
    );
    stmtNodeVec.insert(stmtNodeVec.begin(), v);

    return stmtNodeVec;
}

escargot::Node* parseFunctionExpression(ParseContext* ctx)
{
    /*
    var token, id = null, stricted, firstRestricted, message, tmp,
        params = [], defaults = [], body, previousStrict, node = new Node(),
        isGenerator, previousAllowYield;
     */

    bool previousAllowYield = ctx->m_allowYield;

    expectKeyword(ctx, Function);

    bool isGenerator = match(ctx, Multiply);
    if (isGenerator) {
        lex(ctx);
    }

    ctx->m_allowYield = !isGenerator;
    escargot::Node* id = nullptr;
    RefPtr<ParseStatus> firstRestricted;
    if (!match(ctx, LeftParenthesis)) {
        RefPtr<ParseStatus> token = ctx->m_lookahead;
        if (!ctx->m_strict && !isGenerator && matchKeyword(ctx, Yield)) {
            id = parseNonComputedProperty(ctx);
        } else {
            ctx->m_isFunctionIdentifier = true;
            id = parseVariableIdentifier(ctx);
            ctx->m_isFunctionIdentifier = false;
        }
        ASSERT(id->type() == escargot::NodeType::Identifier);
        if (ctx->m_strict) {
            if (isRestrictedWord(token->m_value)) {
                // tolerateUnexpectedToken(token, Messages.StrictFunctionName);
                throwRestrictedWordUsedError(((escargot::IdentifierNode *)id)->name());
            }
        } else {
            if (isRestrictedWord(token->m_value)) {
                firstRestricted = token;
                // message = Messages.StrictFunctionName;
            } else if (isStrictModeReservedWord(token->m_value)) {
                firstRestricted = token;
                // message = Messages.StrictReservedWord;
            }
        }
    }

    escargot::InternalAtomicStringVector params = parseParams(ctx);
    /*
    tmp = parseParams(firstRestricted);
    params = tmp.params;
    defaults = tmp.defaults;
    stricted = tmp.stricted;
    firstRestricted = tmp.firstRestricted;
    if (tmp.message) {
        message = tmp.message;
    }
    */

    bool previousStrict = ctx->m_strict;
    escargot::Node* body = parseFunctionSourceElements(ctx);

    // return node.finishFunctionExpression(id, params, defaults, body, isGenerator);
    escargot::FunctionExpressionNode* nd;
    if (id)
        nd = new escargot::FunctionExpressionNode(((escargot::IdentifierNode *)id)->name(),
            std::move(params), body, isGenerator, true, ctx->m_strict);
    else
        nd = new escargot::FunctionExpressionNode(escargot::strings->emptyString,
            std::move(params), body, isGenerator, true, ctx->m_strict);
    nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    validateFunctionParamsLazily(ctx, nd, firstRestricted, previousStrict);


    ctx->m_strict = previousStrict;
    ctx->m_allowYield = previousAllowYield;
    return nd;
}


/*
escargot::Node* parseVariableIdentifier(ParseContext* ctx)
{
    ParseStatus* token = lex(ctx);

    if (token->m_type == Token::KeywordToken && token->m_value == u"yield") {
        if (ctx->m_strict) {
            tolerateUnexpectedToken();
        }
        tolerateUnexpectedToken();
        if (!ctx->m_allowYield) {
            throwUnexpectedToken();
        }
    } else if (token->m_type != Token::IdentifierToken) {
        if (ctx->m_strict && token->m_type == Token::KeywordToken && isStrictModeReservedWord(token->m_value)) {
            tolerateUnexpectedToken();
        } else {
            throwUnexpectedToken();
        }
    } else if (sourceType === 'module' && token.type === Token.Identifier && token.value === 'await') {
        tolerateUnexpectedToken(token);
    }

    return new escargot::IdentifierNode(escargot::InternalAtomicString(token->m_value));
}
*/

escargot::Node* parsePattern(ParseContext* ctx, std::vector<RefPtr<ParseStatus> >& params);
escargot::Node* parseLeftHandSideExpression(ParseContext* ctx);
escargot::Node* parseNonComputedProperty(ParseContext* ctx);

escargot::Node* parsePatternWithDefault(ParseContext* ctx, std::vector<RefPtr<ParseStatus> >& params)
{
    // RefPtr<ParseStatus> startToken = ctx->m_lookahead;
    escargot::Node* pattern;
    escargot::Node* right;
    bool previousAllowYield;
    pattern = parsePattern(ctx, params);
    if (match(ctx, Substitution)) {
        lex(ctx);
        previousAllowYield = ctx->m_allowYield;
        ctx->m_allowYield = true;
        right = isolateCoverGrammar(ctx, parseAssignmentExpression);
        ctx->m_allowYield = previousAllowYield;
        // pattern = new WrappingNode(startToken).finishAssignmentPattern(pattern, right);
        pattern = new escargot::AssignmentExpressionSimpleNode(pattern, right);
        pattern->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    }
    return pattern;
}

escargot::Node*  parseArrayPattern(ParseContext* ctx, std::vector<RefPtr<ParseStatus> >& params)
{
    // var node = new Node(), elements = [], rest, restNode;
    expect(ctx, LeftSquareBracket);
    escargot::ExpressionNodeVector elements;
    while (!match(ctx, RightSquareBracket)) {
        if (match(ctx, Comma)) {
            lex(ctx);
            elements.push_back(NULL);
        } else {
            if (match(ctx, PeriodPeriodPeriod)) {
                // TODO implement rest
                // https://developer.mozilla.org/ko/docs/Web/JavaScript/Reference/Functions/rest_parameters
                throw u"ES2015 rest parameter syntax is not supported";
                break;
            } else {
                elements.push_back(parsePatternWithDefault(ctx, params));
            }
            if (!match(ctx, RightSquareBracket)) {
                expect(ctx, Comma);
            }
        }

    }

    expect(ctx, RightSquareBracket);

    escargot::Node* nd = new escargot::ArrayExpressionNode(std::move(elements));
    nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    return nd;
}

// ECMA-262 12.2.5 Array Initializer

escargot::Node* parseArrayInitializer(ParseContext* ctx)
{
    // var elements = [], node = new Node(), restSpread;
    escargot::ExpressionNodeVector elements;

    expect(ctx, LeftSquareBracket);

    while (!match(ctx, RightSquareBracket)) {
        if (match(ctx, Comma)) {
            lex(ctx);
            elements.push_back(NULL);
        } else if (match(ctx, PeriodPeriodPeriod)) {
            RELEASE_ASSERT_NOT_REACHED();

            /*
            restSpread = new Node();
            lex();
            restSpread.finishSpreadElement(inheritCoverGrammar(parseAssignmentExpression));

            if (!match(']')) {
                isAssignmentTarget = isBindingElement = false;
                expect(',');
            }
            elements.push(restSpread);
             */

        } else {
            elements.push_back(inheritCoverGrammar(ctx, parseAssignmentExpression));

            if (!match(ctx, RightSquareBracket)) {
                expect(ctx, Comma);
            }
        }
    }

    lex(ctx);

    // return node.finishArrayExpression(elements);
    escargot::Node* nd = new escargot::ArrayExpressionNode(std::move(elements));
    nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    return nd;
}

escargot::Node* parseObjectPropertyKey(ParseContext* ctx)
{
    // var token, node = new Node(), expr;

    RefPtr<ParseStatus> token = lex(ctx);

    // Note: This function is called only from parseObjectProperty(), where
    // EOF and Punctuator tokens are already filtered out.
    escargot::Node* nd;
    switch (token->m_type) {
    case Token::StringLiteralToken:
        if (ctx->m_strict && token->m_octal) {
            // tolerateUnexpectedToken(token, Messages.StrictOctalLiteral);
            throwOctalLiteralUsedError();
        }
        {
            nd = new escargot::LiteralNode(token->m_value.toESString());
            nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
        }
        return nd;
    case Token::NumericLiteralToken:
        if (ctx->m_strict && token->m_octal) {
            // tolerateUnexpectedToken(token, Messages.StrictOctalLiteral);
            throwOctalLiteralUsedError();
        }
        nd = new escargot::LiteralNode(escargot::ESValue(token->m_valueNumber));
        nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
        return nd;
    case Token::IdentifierToken:
    case Token::BooleanLiteralToken:
    case Token::NullLiteralToken:
    case Token::KeywordToken:
        {
            // return node.finishIdentifier(token.value);
            nd = new escargot::LiteralNode(token->m_value.toESString());
            nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
            return nd;
        }
    case Token::PunctuatorToken:
        if (token->m_punctuatorsKind == LeftSquareBracket) {
            escargot::Node* expr = isolateCoverGrammar(ctx, parseAssignmentExpression);
            expect(ctx, RightSquareBracket);
            return expr;
        }
        break;
    default:
        break;
    };
    throwUnexpectedToken(token, u"Expected a property name");
    RELEASE_ASSERT_NOT_REACHED();
}

escargot::PropertyNode* parsePropertyPattern(ParseContext* ctx, std::vector<RefPtr<ParseStatus> >& params)
{
    // var node = new Node(),
    escargot::Node* key; // , keyToken,
    RefPtr<ParseStatus> keyToken;
    bool computed = match(ctx, LeftSquareBracket);
    escargot::Node* init;
    // , init;
    if (ctx->m_lookahead->m_type == Token::IdentifierToken) {
        keyToken = ctx->m_lookahead;
        key = parseVariableIdentifier(ctx);
        if (match(ctx, Substitution)) {
            params.push_back(keyToken);
            lex(ctx);
            init = parseAssignmentExpression(ctx);
            // AssignmentExpressionNode(Node* left, Node* right, ESString* oper)
            escargot::Node* value = new escargot::AssignmentExpressionSimpleNode(key, init);
            value->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
            escargot::PropertyNode* nd = new escargot::PropertyNode(key, value , escargot::PropertyNode::Kind::Init);
            nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
            return nd;
            /*return node.finishProperty(
                'init', key, false,
                new WrappingNode(keyToken).finishAssignmentPattern(key, init), false, false);
             */
        } else if (!match(ctx, Colon)) {
            params.push_back(keyToken);
            // return node.finishProperty('init', key, false, key, false, true);
            escargot::PropertyNode* nd = new escargot::PropertyNode(key, key , escargot::PropertyNode::Kind::Init);
            nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
            return nd;
        }
    } else {
        // key = parseObjectPropertyKey(ctx, params);
        key = parseObjectPropertyKey(ctx);
    }
    expect(ctx, Colon);
    init = parsePatternWithDefault(ctx, params);
    // return node.finishProperty('init', key, computed, init, false, false);
    escargot::PropertyNode* nd = new escargot::PropertyNode(key, init , escargot::PropertyNode::Kind::Init);
    nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    return nd;
}

escargot::Node* parseObjectPattern(ParseContext* ctx, std::vector<RefPtr<ParseStatus> >& params)
{
    // var node = new Node(), properties = [];
    escargot::PropertiesNodeVector properties;
    expect(ctx, LeftBrace);

    while (!match(ctx, RightBrace)) {
        properties.push_back(parsePropertyPattern(ctx, params));
        if (!match(ctx, RightBrace)) {
            expect(ctx, Comma);
        }
    }

    lex(ctx);

    escargot::Node* nd = new escargot::ObjectExpressionNode(std::move(properties));
    nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    return nd;
}

escargot::Node* parsePattern(ParseContext* ctx, std::vector<RefPtr<ParseStatus> >& params)
{
    if (match(ctx, LeftSquareBracket)) {
        if (ctx->m_inCatch) {
            throwUnexpectedToken(lex(ctx), IdentifierToken, u"as catch target.");
        } else {
            throwUnexpectedToken(lex(ctx), IdentifierToken);
        }
        // return parseArrayPattern(ctx, params);
    } else if (match(ctx, LeftBrace)) {
        if (ctx->m_inCatch) {
            throwUnexpectedToken(lex(ctx), IdentifierToken, u"as catch target.");
        } else {
            throwUnexpectedToken(lex(ctx), IdentifierToken);
        }
        // return parseObjectPattern(ctx, params);
    }
    params.push_back(ctx->m_lookahead);
    return parseVariableIdentifier(ctx);
}

struct ScanRegExpBodyResult {
    ParserString m_value;
    ParserString m_literal;
};

ScanRegExpBodyResult* scanRegExpBody(ParseContext* ctx)
{
    char16_t ch;
    ParserString str;
    // , str, classMarker, terminated, body;

    ch = ctx->m_sourceString->charAt(ctx->m_index);
    // assert(ch === '/', 'Regular expression literal must start with a slash');
    ASSERT(ch == '/');
    str = ctx->m_sourceString->charAt(ctx->m_index++);

    const char16_t* unterminatedRegExp = u"unterminated regular expression";

    bool classMarker = false;
    bool terminated = false;
    while (ctx->m_index < ctx->m_length) {
        ch = ctx->m_sourceString->charAt(ctx->m_index++);
        str += ch;
        if (ch == '\\') {
            ch = ctx->m_sourceString->charAt(ctx->m_index++);
            // ECMA-262 7.8.5
            if (isLineTerminator(ch)) {
                // throwUnexpectedToken(null, Messages.UnterminatedRegExp);
                throwEsprimaException(unterminatedRegExp);
            }
            str += ch;
        } else if (isLineTerminator(ch)) {
            // throwUnexpectedToken(null, Messages.UnterminatedRegExp);
            throwEsprimaException(unterminatedRegExp);
        } else if (classMarker) {
            if (ch == ']') {
                classMarker = false;
            }
        } else {
            if (ch == '/') {
                terminated = true;
                break;
            } else if (ch == '[') {
                classMarker = true;
            }
        }
    }

    if (!terminated) {
        // throwUnexpectedToken(null, Messages.UnterminatedRegExp);
        throwEsprimaException(unterminatedRegExp);
    }

    // Exclude leading and trailing slash.
    ParserString body = str.substr(1, str.length() - 2);
    ScanRegExpBodyResult* result = new ScanRegExpBodyResult();
    result->m_value = std::move(body);
    result->m_literal = std::move(str);
    return result;
    /*
    return {
        value: std::move(body),
                literal: str
    };
    */
}

struct ScanRegExpFlagsResult {
    ParserString m_value;
    ParserString m_literal;
};


ScanRegExpFlagsResult* scanRegExpFlags(ParseContext* ctx)
{
    // var ch, str, flags, restore;
    char16_t ch;

    ParserString str;
    ParserString flags;
    size_t restore;
    while (ctx->m_index < ctx->m_length) {
        ch = ctx->m_sourceString->charAt(ctx->m_index);
        if (!isIdentifierPart(ch)) {
            break;
        }

        ++ctx->m_index;
        if (ch == '\\' && ctx->m_index < ctx->m_length) {
            ch = ctx->m_sourceString->charAt(ctx->m_index);
            if (ch == 'u') {
                ++ctx->m_index;
                restore = ctx->m_index;
                ch = scanHexEscape(ctx, 'u');
                if (ch) {
                    flags += ch;
                    str += '\\';
                    str += 'u';
                    for (; restore < ctx->m_index; ++restore) {
                        str += ctx->m_sourceString->charAt(restore);
                    }
                } else {
                    ctx->m_index = restore;
                    flags += 'u';
                    str += '\\';
                    str += 'u';
                }
                throw u"Invalid regular expression flags";
            } else {
                str += '\\';
                throw u"Invalid regular expression flags";
            }
        } else {
            flags += ch;
            str += ch;
        }
    }

    ScanRegExpFlagsResult* result = new ScanRegExpFlagsResult();
    result->m_value = std::move(flags);
    result->m_literal = std::move(str);
    return result;
    /*
    return {
        value: flags,
        literal: str
    };
    */
}

PassRefPtr<ParseStatus> scanRegExp(ParseContext* ctx)
{
    ctx->m_scanning = true;
    // var start, body, flags, value;
    size_t start;

    ctx->m_lookahead = NULL;
    skipComment(ctx);
    start = ctx->m_index;

    ScanRegExpBodyResult* body = scanRegExpBody(ctx);
    ScanRegExpFlagsResult* flags = scanRegExpFlags(ctx);
    // value = testRegExp(body.value, flags.value);
    ctx->m_scanning = false;
    /*
     if (extra.tokenize) {
        return {
            type: Token.RegularExpression,
            value: value,
            regex: {
                pattern: body.value,
                flags: flags.value
            },
            lineNumber: lineNumber,
            lineStart: lineStart,
            start: start,
            end: index
        };
    }
    */

    ParseStatus* ps = new ParseStatus();
    ps->m_regexBody = std::move(body->m_value);
    ps->m_regexFlag = std::move(flags->m_value);
    delete body;
    delete flags;
    // ps->m_value = value;
    ps->m_start = start;
    ps->m_end = ctx->m_index;
    /*
    return {
        literal: body.literal + flags.literal,
        value: value,
        regex: {
            pattern: body.value,
            flags: flags.value
        },
        start: start,
        end: index
    };
    */
    return adoptRef(ps);
}

bool lookaheadPropertyName(ParseContext* ctx)
{
    switch (ctx->m_lookahead->m_type) {
    case Token::IdentifierToken:
    case Token::StringLiteralToken:
    case Token::BooleanLiteralToken:
    case Token::NullLiteralToken:
    case Token::NumericLiteralToken:
    case Token::KeywordToken:
        return true;
    case Token::PunctuatorToken:
        return ctx->m_lookahead->m_punctuatorsKind == LeftSquareBracket;
    default:
        return false;
    }
}

escargot::Node* parsePropertyFunction(ParseContext* ctx, escargot::InternalAtomicStringVector& vec, escargot::Node* key/*paramInfo, isGenerator*/)
{
    // var previousStrict, body;

    ctx->m_isAssignmentTarget = ctx->m_isBindingElement = false;
    RefPtr<ParseStatus> firstRestricted;
    bool previousStrict = ctx->m_strict;
    escargot::Node* body = isolateCoverGrammar(ctx, parseFunctionSourceElements);

    /*
    // TODO
    if (strict && paramInfo.firstRestricted) {
        tolerateUnexpectedToken(paramInfo.firstRestricted, paramInfo.message);
    }
    if (strict && paramInfo.stricted) {
        tolerateUnexpectedToken(paramInfo.stricted, paramInfo.message);
    }
    */

    // return node.finishFunctionExpression(null, paramInfo.params, paramInfo.defaults, body, isGenerator);
    const escargot::InternalAtomicString& id = (key->isIdentifier()) ? ((escargot::IdentifierNode*)key)->name() :
        key->isLiteral() ? escargot::InternalAtomicString(((escargot::LiteralNode*)key)->value().toString()->utf8Data()) : escargot::strings->emptyString;
    escargot::FunctionExpressionNode* nd = new escargot::FunctionExpressionNode(id, std::move(vec), body, false, true, ctx->m_strict);
    validateFunctionParamsLazily(ctx, nd, firstRestricted, previousStrict);
    ctx->m_strict = previousStrict;
    nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    return nd;
}

escargot::Node* parsePropertyMethodFunction(ParseContext* ctx, escargot::Node* key)
{
    bool previousAllowYield = ctx->m_allowYield;
    ctx->m_allowYield = false;
    escargot::InternalAtomicStringVector params = parseParams(ctx);
    ctx->m_allowYield = previousAllowYield;

    ctx->m_allowYield = false;
    escargot::InternalAtomicStringVector vec;
    escargot::Node* method = parsePropertyFunction(ctx, params, key);
    ctx->m_allowYield = previousAllowYield;

    escargot::Node* nd = new escargot::PropertyNode(key, method, escargot::PropertyNode::Init);
    nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    return nd;
}

// This function is to try to parse a MethodDefinition as defined in 14.3. But in the case of object literals,
// it might be called at a position where there is in fact a short hand identifier pattern or a data property.
// This can only be determined after we consumed up to the left parentheses.
// 
// In order to avoid back tracking, it returns `null` if the position is not a MethodDefinition and the caller
// is responsible to visit other options.
escargot::Node* tryParseMethodDefinition(ParseContext* ctx, ParseStatus* token, escargot::Node* key, bool computed)
{
    bool previousAllowYield = ctx->m_allowYield;
    // var value, options, methodNode, params,
    // previousAllowYield = state.allowYield;

    if (token->m_type == Token::IdentifierToken) {
        // check for `get` and `set`;

        if (token->m_value == u"get" && lookaheadPropertyName(ctx)) {
            computed = match(ctx, LeftSquareBracket);
            key = parseObjectPropertyKey(ctx);
            // methodNode = new Node();

            ctx->m_allowYield = false;
            escargot::InternalAtomicStringVector vec = parseParams(ctx);
            if (vec.size() != 0)
                throwEsprimaException(u"Getter functions must have zero argument");
            escargot::Node* value = parsePropertyFunction(ctx, vec, key
                /*
                {
                    params: [],
                    defaults: [],
                    stricted: null,
                    firstRestricted: null,
                    message: null
                }, false
                */
                );
            ctx->m_allowYield = previousAllowYield;

            // return node.finishProperty('get', key, computed, value, false, false);
            escargot::Node* nd = new escargot::PropertyNode(key, value, escargot::PropertyNode::Get);
            nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
            return nd;
        } else if (token->m_value == u"set" && lookaheadPropertyName(ctx)) {
            computed = match(ctx, LeftSquareBracket);
            key = parseObjectPropertyKey(ctx);
            // methodNode = new Node();
            /*
            options = {
                params: [],
                defaultCount: 0,
                defaults: [],
                firstRestricted: null,
                paramSet: {}
            };
            */
            escargot::InternalAtomicStringVector vec = parseParams(ctx);
            if (vec.size() != 1)
                throwEsprimaException(u"Setter functions must have one argument");

            ctx->m_allowYield = false;
            escargot::Node* value = parsePropertyFunction(ctx, vec, key/*methodNode, options, false*/);
            ctx->m_allowYield = previousAllowYield;

            // return node.finishProperty('set', key, computed, value, false, false);
            escargot::Node* nd = new escargot::PropertyNode(key, value, escargot::PropertyNode::Set);
            nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
            return nd;
        }
    } else if (token->m_type == Token::PunctuatorToken && token->m_punctuatorsKind == Multiply && lookaheadPropertyName(ctx)) {
        computed = match(ctx, LeftSquareBracket);
        key = parseObjectPropertyKey(ctx);
        // methodNode = new Node();

        ctx->m_allowYield = false;
        escargot::InternalAtomicStringVector params = parseParams(ctx);
        ctx->m_allowYield = previousAllowYield;

        ctx->m_allowYield = false;
        escargot::Node* value = parsePropertyFunction(ctx, params, key);
        ctx->m_allowYield = previousAllowYield;

        escargot::Node* nd = new escargot::PropertyNode(key, value, escargot::PropertyNode::Init);
        nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
        return nd;
        // return node.finishProperty('init', key, computed, value, true, false);
    }

    if (key && match(ctx, LeftParenthesis)) {
        return parsePropertyMethodFunction(ctx, key);
    }

    // Not a MethodDefinition.
    return NULL;
}

escargot::Node* parseObjectProperty(ParseContext* ctx, bool& hasProto)
{
    RefPtr<ParseStatus> token = ctx->m_lookahead;
    // , node = new Node(), computed, key, maybeMethod, proto, value;
    bool proto;
    escargot::Node* key = nullptr;
    bool computed = match(ctx, LeftSquareBracket);
    if (match(ctx, Multiply)) {
        lex(ctx);
    } else {
        key = parseObjectPropertyKey(ctx);
    }

    escargot::Node* maybeMethod = tryParseMethodDefinition(ctx, token.get(), key, computed);
    if (maybeMethod) {
        return maybeMethod;
    }

    if (!key) {
        throwEsprimaException();
        RELEASE_ASSERT_NOT_REACHED();
        // throwUnexpectedToken(lookahead);
    }

    // Check for duplicated __proto__
    if (!computed) {
        // proto = (key.type === Syntax.Identifier && key.name === '__proto__') ||
        // (key.type === Syntax.Literal && key.value === '__proto__');
        proto = (key->type() == escargot::NodeType::Identifier && *((escargot::IdentifierNode *)key)->nonAtomicName() == "__proto__")
            || (key->type() == escargot::NodeType::Literal && ((escargot::LiteralNode *)key)->value().equalsTo(escargot::ESVMInstance::currentInstance()->strings().__proto__.string()));
        if (hasProto && proto) {
            // tolerateError(Messages.DuplicateProtoProperty);
            tolerateError(u"Messages.DuplicateProtoProperty");
        }
        hasProto |= proto;
    }

    if (match(ctx, Colon)) {
        lex(ctx);
        escargot::Node* value = inheritCoverGrammar(ctx, parseAssignmentExpression);
        // return node.finishProperty('init', key, computed, value, false, false);
        escargot::Node* nd = new escargot::PropertyNode(key, value, escargot::PropertyNode::Kind::Init);
        nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
        return nd;
    } else {
        throwEsprimaException();
    }

    if (token->m_type == Token::IdentifierToken) {
        if (match(ctx, Substitution)) {
            ctx->m_firstCoverInitializedNameError = ctx->m_lookahead;
            lex(ctx);
            escargot::Node* value = isolateCoverGrammar(ctx, parseAssignmentExpression);
            // return node.finishProperty('init', key, computed,
            // new WrappingNode(token).finishAssignmentPattern(key, value), false, true);
            escargot::Node* nd = new escargot::PropertyNode(key, new escargot::AssignmentExpressionSimpleNode(key, value), escargot::PropertyNode::Kind::Init);
            nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
            return nd;
        }
        // return node.finishProperty('init', key, computed, key, false, true);
        escargot::Node* nd = new escargot::PropertyNode(key, key, escargot::PropertyNode::Kind::Init);
        nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
        return nd;
    }

    // throwUnexpectedToken(lookahead);
    throwEsprimaException();
    RELEASE_ASSERT_NOT_REACHED();
}

escargot::Node* parseObjectInitializer(ParseContext* ctx)
{
    // var properties = [], hasProto = {value: false}, node = new Node();
    escargot::PropertiesNodeVector properties;
    bool hasProto = false;
    expect(ctx, LeftBrace);
    std::unordered_map<escargot::ESString*, escargot::PropertyNode::Kind> keyStrings;

    while (!match(ctx, RightBrace)) {
        escargot::PropertyNode* p = (escargot::PropertyNode *)parseObjectProperty(ctx, hasProto);
        if (!p->isValidPropertyNode())
            throw u"Object initializer should be either identifier or literal";
        escargot::ESString* keyString = p->keyString();
        auto previous = keyStrings.find(keyString);
        if (previous != keyStrings.end()) {
            if (ctx->m_strict && (previous->second == escargot::PropertyNode::Kind::Init) && (p->kind() == escargot::PropertyNode::Kind::Init))
                throwEsprimaException(keyString);
            if ((previous->second == escargot::PropertyNode::Kind::Init) && (p->kind() != escargot::PropertyNode::Kind::Init))
                throwEsprimaException(keyString);
            if ((previous->second != escargot::PropertyNode::Kind::Init) && (p->kind() == escargot::PropertyNode::Kind::Init))
                throwEsprimaException(keyString);
            if (((previous->second == escargot::PropertyNode::Kind::Get) && (p->kind() == escargot::PropertyNode::Kind::Get))
                || ((previous->second == escargot::PropertyNode::Kind::Set) && (p->kind() == escargot::PropertyNode::Kind::Set)))
                throwEsprimaException(keyString);
        }
        keyStrings.insert(std::make_pair(keyString, p->kind()));

        properties.push_back(p);

        if (!match(ctx, RightBrace)) {
            expectCommaSeparator(ctx);
        }
    }

    expect(ctx, RightBrace);

    // return node.finishObjectExpression(properties);
    escargot::Node* nd = new escargot::ObjectExpressionNode(std::move(properties));
    nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    return nd;
}

void reinterpretExpressionAsPattern(ParseContext* ctx, escargot::Node* expr)
{
    /*
    var i;
    switch (expr.type) {
    case Syntax.Identifier:
    case Syntax.MemberExpression:
    case Syntax.RestElement:
    case Syntax.AssignmentPattern:
        break;
    case Syntax.SpreadElement:
        expr.type = Syntax.RestElement;
        reinterpretExpressionAsPattern(expr.argument);
        break;
    case Syntax.ArrayExpression:
        expr.type = Syntax.ArrayPattern;
        for (i = 0; i < expr.elements.length; i++) {
            if (expr.elements[i] !== null) {
                reinterpretExpressionAsPattern(expr.elements[i]);
            }
        }
        break;
    case Syntax.ObjectExpression:
        expr.type = Syntax.ObjectPattern;
        for (i = 0; i < expr.properties.length; i++) {
            reinterpretExpressionAsPattern(expr.properties[i].value);
        }
        break;
    case Syntax.AssignmentExpression:
        expr.type = Syntax.AssignmentPattern;
        reinterpretExpressionAsPattern(expr.left);
        break;
    default:
        // Allow other node type for tolerant parsing.
        break;
    }
    */
}

// ECMA-262 12.2.9 Template Literals

void parseTemplateElement(ParseContext* ctx/*, option*/)
{
    RELEASE_ASSERT_NOT_REACHED();

    /*
    var node, token;

    if (lookahead.type !== Token.Template || (option.head && !lookahead.head)) {
        throwUnexpectedToken();
    }

    node = new Node();
    token = lex();

    return node.finishTemplateElement({ raw: token.value.raw, cooked: token.value.cooked }, token.tail);
    */
}

escargot::Node* parseTemplateLiteral(ParseContext* ctx)
{
    throw u"ES2015 Template string is not supported";
}

// ECMA-262 12.2.10 The Grouping Operator

escargot::Node* parseGroupExpression(ParseContext* ctx)
{
    expect(ctx, LeftParenthesis);

    if (match(ctx, RightParenthesis)) {
        // arrow function
        throw u"ES2015 arrow function expression is not supported";
    }

    RefPtr<ParseStatus> startToken = ctx->m_lookahead;
    if (match(ctx, PeriodPeriodPeriod)) {
        // rest element
        throw u"ES2015 rest parameter syntax is not supported";
    }

    ctx->m_isBindingElement = true;
    escargot::Node* expr = inheritCoverGrammar(ctx, parseAssignmentExpression);

    if (match(ctx, Comma)) {
        escargot::ExpressionNodeVector expressions;
        ctx->m_isAssignmentTarget = false;
        expressions.push_back(expr);

        while (ctx->m_startIndex < ctx->m_length) {
            if (!match(ctx, Comma)) {
                break;
            }
            lex(ctx);

            if (match(ctx, PeriodPeriodPeriod)) {
                // rest element
                throw u"ES2015 rest parameter syntax is not supported";
                /*
                if (!isBindingElement) {
                    throwUnexpectedToken(lookahead);
                }
                expressions.push(parseRestElement(params));
                expect(')');
                if (!match('=>')) {
                    expect('=>');
                }
                isBindingElement = false;
                for (i = 0; i < expressions.length; i++) {
                    reinterpretExpressionAsPattern(expressions[i]);
                }
                return {
                    type: PlaceHolders.ArrowParameterPlaceHolder,
                    params: expressions
                };
                */
            }

            expressions.push_back(inheritCoverGrammar(ctx, parseAssignmentExpression));
        }

        expr = new escargot::SequenceExpressionNode(std::move(expressions));
        expr->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    }


    expect(ctx, RightParenthesis);

    if (match(ctx, Arrow)) {
        // arrow function
        throw u"ES2015 arrow function expression is not supported";
        /*
        if (expr.type === Syntax.Identifier && expr.name === 'yield') {
            return {
                type: PlaceHolders.ArrowParameterPlaceHolder,
                params: [expr]
            };
        }

        if (!isBindingElement) {
            throwUnexpectedToken(lookahead);
        }

        if (expr.type === Syntax.SequenceExpression) {
            for (i = 0; i < expr.expressions.length; i++) {
                reinterpretExpressionAsPattern(expr.expressions[i]);
            }
        } else {
            reinterpretExpressionAsPattern(expr);
        }

        expr = {
            type: PlaceHolders.ArrowParameterPlaceHolder,
            params: expr.type === Syntax.SequenceExpression ? expr.expressions : [expr]
        };
        */
    }
    ctx->m_isBindingElement = false;
    return expr;

    /*
    var expr, expressions, startToken, i, params = [];

    expect(ctx, {'('});

    if (match(ctx, {')'})) {
        lex(ctx);
        if (!match(ctx. {'=>'})) {
            expect(ctx, {'=>'});
        }
        return {
            type: PlaceHolders.ArrowParameterPlaceHolder,
            params: [],
            rawParams: []
        };
    }

    startToken = lookahead;
    if (match('...')) {
        expr = parseRestElement(params);
        expect(')');
        if (!match('=>')) {
            expect('=>');
        }
        return {
            type: PlaceHolders.ArrowParameterPlaceHolder,
            params: [expr]
        };
    }

    isBindingElement = true;
    expr = inheritCoverGrammar(parseAssignmentExpression);

    if (match(',')) {
        isAssignmentTarget = false;
        expressions = [expr];

        while (startIndex < length) {
            if (!match(',')) {
                break;
            }
            lex();

            if (match('...')) {
                if (!isBindingElement) {
                    throwUnexpectedToken(lookahead);
                }
                expressions.push(parseRestElement(params));
                expect(')');
                if (!match('=>')) {
                    expect('=>');
                }
                isBindingElement = false;
                for (i = 0; i < expressions.length; i++) {
                    reinterpretExpressionAsPattern(expressions[i]);
                }
                return {
                    type: PlaceHolders.ArrowParameterPlaceHolder,
                    params: expressions
                };
            }

            expressions.push(inheritCoverGrammar(parseAssignmentExpression));
        }

        expr = new WrappingNode(startToken).finishSequenceExpression(expressions);
    }


    expect(')');

    if (match('=>')) {
        if (expr.type === Syntax.Identifier && expr.name === 'yield') {
            return {
                type: PlaceHolders.ArrowParameterPlaceHolder,
                params: [expr]
            };
        }

        if (!isBindingElement) {
            throwUnexpectedToken(lookahead);
        }

        if (expr.type === Syntax.SequenceExpression) {
            for (i = 0; i < expr.expressions.length; i++) {
                reinterpretExpressionAsPattern(expr.expressions[i]);
            }
        } else {
            reinterpretExpressionAsPattern(expr);
        }

        expr = {
            type: PlaceHolders.ArrowParameterPlaceHolder,
            params: expr.type === Syntax.SequenceExpression ? expr.expressions : [expr]
        };
    }
    isBindingElement = false;
    return expr;
     */

}

// ECMA-262 12.2 Primary Expressions

escargot::Node* parsePrimaryExpression(ParseContext* ctx)
{
    // var type, token, expr, node;
    RefPtr<ParseStatus> token;
    escargot::Node* expr = nullptr;
    escargot::Node* node;

    if (match(ctx, LeftParenthesis)) {
        ctx->m_isBindingElement = false;
        return inheritCoverGrammar(ctx, parseGroupExpression);
    }

    if (match(ctx, LeftSquareBracket)) {
        return inheritCoverGrammar(ctx, parseArrayInitializer);
    }

    if (match(ctx, LeftBrace)) {
        return inheritCoverGrammar(ctx, parseObjectInitializer);
    }

    Token type = ctx->m_lookahead->m_type;
    // node = new Node();

    if (type == Token::IdentifierToken) {
        /*
         if (sourceType === 'module' && lookahead.value === 'await') {
        tolerateUnexpectedToken(lookahead);
        }*/
        auto ll = lex(ctx);
        if (ctx->m_strict && isStrictModeReservedWord(ll->m_value))
            throwUnexpectedToken(ll);
        expr = new escargot::IdentifierNode(ll->m_value.toInternalAtomicString());
        expr->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
        // expr = node.finishIdentifier(lex().value);
    } else if (type == Token::StringLiteralToken || type == Token::NumericLiteralToken) {
        ctx->m_isAssignmentTarget = ctx->m_isBindingElement = false;
        if (ctx->m_strict && ctx->m_lookahead->m_octal) {
            // tolerateUnexpectedToken(lookahead, Messages.StrictOctalLiteral);
            throwOctalLiteralUsedError();
        }
        expr = finishLiteralNode(ctx, lex(ctx));
    } else if (type == Token::KeywordToken) {
        if (!ctx->m_strict && ctx->m_allowYield && matchKeyword(ctx, Yield)) {
            return parseNonComputedProperty(ctx);
        }
        ctx->m_isAssignmentTarget = ctx->m_isBindingElement = false;
        if (matchKeyword(ctx, Function)) {
            return parseFunctionExpression(ctx);
        }
        if (matchKeyword(ctx, This)) {
            lex(ctx);
            escargot::Node* nd = new escargot::ThisExpressionNode();
            nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
            return nd;
        }
        if (matchKeyword(ctx, Class)) {
            throw u"ES2015 class feature is not supported";
            // return parseClassExpression();
        }
        token = lex(ctx);
        throwUnexpectedToken(token);
    } else if (type == Token::BooleanLiteralToken) {
        ctx->m_isAssignmentTarget = ctx->m_isBindingElement = false;
        token = lex(ctx);
        bool token_value = (token->m_value == u"true");
        expr = new escargot::LiteralNode(escargot::ESValue(token_value));
        expr->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    } else if (type == Token::NullLiteralToken) {
        ctx->m_isAssignmentTarget = ctx->m_isBindingElement = false;
        token = lex(ctx);
        expr = new escargot::LiteralNode(escargot::ESValue(escargot::ESValue::ESNull));
        expr->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    } else if (match(ctx, Divide) || match(ctx, DivideEqual)) {
        ctx->m_isAssignmentTarget = ctx->m_isBindingElement = false;
        ctx->m_index = ctx->m_startIndex;
        /*
        if (typeof extra.tokens !== 'undefined') {
            token = collectRegex(ctx);
        } else {
            token = scanRegExp(ctx);
        }
        */
        token = scanRegExp(ctx);
        lex(ctx);
        // expr = node.finishLiteral(token);

        escargot::ESRegExpObject::Option option = escargot::ESRegExpObject::parseOption(token->m_regexFlag.toESString());
        expr = new escargot::RegExpLiteralNode(token->m_regexBody.toESString(), option);
        expr->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
        // parsedNode = new LiteralNode(ESRegExpObject::create(source, (escargot::ESRegExpObject::Option)f, escargot::ESVMInstance::currentInstance()->globalObject()->regexpPrototype()));
    } else if (type == Token::TemplateToken) {
        expr = parseTemplateLiteral(ctx);
    } else {
        token = lex(ctx);
        throwUnexpectedToken(token);
    }

    ASSERT(expr);
    return expr;
}

// ECMA-262 12.3 Left-Hand-Side Expressions

escargot::ArgumentVector parseArguments(ParseContext* ctx)
{
    // var args = [], expr;
    escargot::Node* expr;

    escargot::ArgumentVector args;

    expect(ctx, LeftParenthesis);

    if (!match(ctx, RightParenthesis)) {
        while (ctx->m_startIndex < ctx->m_length) {
            if (match(ctx, PeriodPeriodPeriod)) {
                throw u"ES2015 Spread operator is not supported";
            } else {
                expr = isolateCoverGrammar(ctx, parseAssignmentExpression);
            }
            args.push_back(expr);
            if (match(ctx, RightParenthesis)) {
                break;
            }
            expectCommaSeparator(ctx);
        }
    }

    expect(ctx, RightParenthesis);

    return args;
}

escargot::Node* parseNonComputedProperty(ParseContext* ctx)
{
    // var token, node = new Node();

    RefPtr<ParseStatus> token = lex(ctx);

    if (!isIdentifierName(token.get())) {
        throwUnexpectedToken(token, u"Expected a property name");
    }

    // return node.finishIdentifier(token.value);
    escargot::Node* nd = new escargot::IdentifierNode(token->m_value.toInternalAtomicString());
    nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    return nd;
}

escargot::Node* parseNonComputedMember(ParseContext* ctx)
{
    expect(ctx, Period);

    return parseNonComputedProperty(ctx);
}

escargot::Node* parseComputedMember(ParseContext* ctx)
{
    escargot::Node* expr;

    expect(ctx, LeftSquareBracket);

    expr = isolateCoverGrammar(ctx, parseExpression);

    expect(ctx, RightSquareBracket);

    return expr;
}

// ECMA-262 12.3.3 The new Operator

escargot::Node* parseNewExpression(ParseContext* ctx)
{
    // var callee, args, node = new Node();
    escargot::Node* callee;

    expectKeyword(ctx, New);

    if (match(ctx, Period)) {
        lex(ctx);
        if (ctx->m_lookahead->m_type == Token::IdentifierToken && ctx->m_lookahead->m_value == u"target") {
            if (ctx->m_inFunctionBody) {
                lex(ctx);
                throw u"ES2016 new.target syntax is not supported";
                // return node.finishMetaProperty('new', 'target');
            }
        }
        throwEsprimaException();
    }

    callee = isolateCoverGrammar(ctx, parseLeftHandSideExpression);
    escargot::ArgumentVector args;
    if (match(ctx, LeftParenthesis)) {
        args = parseArguments(ctx);
    }

    ctx->m_isAssignmentTarget = ctx->m_isBindingElement = false;

    // return node.finishNewExpression(callee, args);
    escargot::Node* nd = new escargot::NewExpressionNode(callee, std::move(args));
    nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    return nd;
}

// ECMA-262 12.3.4 Function Calls

escargot::Node* parseLeftHandSideExpressionAllowCall(ParseContext* ctx)
{
    // var quasi, expr, args, property, startToken, ;
    escargot::Node* quasi;
    escargot::Node* expr;
    escargot::Node* property;
    escargot::ArgumentVector args;
    bool previousAllowIn = ctx->m_allowIn;

    // RefPtr<ParseStatus> startToken = ctx->m_lookahead;
    ctx->m_allowIn = true;

    if (matchKeyword(ctx, Super) && ctx->m_inFunctionBody) {
        throw u"ES2015 super expression is not supported";
        /*
        expr = new Node();
        lex();
        expr = expr.finishSuper();
        if (!match('(') && !match('.') && !match('[')) {
            throwUnexpectedToken(lookahead);
        }*/
    } else {
        expr = inheritCoverGrammar(ctx, matchKeyword(ctx, New) ? parseNewExpression : parsePrimaryExpression);
    }

    for (;;) {
        if (match(ctx, Period)) {
            ctx->m_isBindingElement = false;
            ctx->m_isAssignmentTarget = true;
            property = parseNonComputedMember(ctx);
            // expr = new WrappingNode(startToken).finishMemberExpression('.', expr, property);
            ASSERT(property->type() == escargot::NodeType::Identifier);
            expr = new escargot::MemberExpressionNode(expr, property, false);
            expr->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
        } else if (match(ctx, LeftParenthesis)) {
            ctx->m_isBindingElement = false;
            ctx->m_isAssignmentTarget = false;
            args = parseArguments(ctx);
            // expr = new WrappingNode(startToken).finishCallExpression(expr, args);
            expr = new escargot::CallExpressionNode(expr, std::move(args));
            expr->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
        } else if (match(ctx, LeftSquareBracket)) {
            ctx->m_isBindingElement = false;
            ctx->m_isAssignmentTarget = true;
            property = parseComputedMember(ctx);
            // expr = new WrappingNode(startToken).finishMemberExpression('[', expr, property);
            expr = new escargot::MemberExpressionNode(expr, property, true);
            expr->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
        } else if (ctx->m_lookahead->m_type == Token::TemplateToken && ctx->m_lookahead->m_head) {
            quasi = parseTemplateLiteral(ctx);
            // expr = new WrappingNode(startToken).finishTaggedTemplateExpression(expr, quasi);
            RELEASE_ASSERT_NOT_REACHED();
        } else {
            break;
        }
    }
    ctx->m_allowIn = previousAllowIn;

    return expr;
}

// ECMA-262 12.3 Left-Hand-Side Expressions

escargot::Node* parseLeftHandSideExpression(ParseContext* ctx)
{
    // var quasi, expr, property, startToken;
    // assert(state.allowIn, 'callee of new expression always allow in keyword.');
    ASSERT(ctx->m_allowIn);
    escargot::Node* expr;
    escargot::Node* quasi;
    escargot::Node* property;
    // RefPtr<ParseStatus> startToken = ctx->m_lookahead;

    if (matchKeyword(ctx, Super) && ctx->m_inFunctionBody) {
        RELEASE_ASSERT_NOT_REACHED();
        /*
        expr = new Node();
        lex();
        expr = expr.finishSuper();
        if (!match('[') && !match('.')) {
            throwUnexpectedToken(lookahead);
        }
        */
    } else {
        expr = inheritCoverGrammar(ctx, matchKeyword(ctx, New) ? parseNewExpression : parsePrimaryExpression);
    }

    for (;;) {
        if (match(ctx, LeftSquareBracket)) {
            ctx->m_isBindingElement = false;
            ctx->m_isAssignmentTarget = true;
            property = parseComputedMember(ctx);
            // expr = new WrappingNode(startToken).finishMemberExpression('[', expr, property);
            // computed = accessor === '[';
            expr = new escargot::MemberExpressionNode(expr, property, true);
            expr->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
        } else if (match(ctx, Period)) {
            ctx->m_isBindingElement = false;
            ctx->m_isAssignmentTarget = true;
            property = parseNonComputedMember(ctx);
            // expr = new WrappingNode(startToken).finishMemberExpression('.', expr, property);
            ASSERT(property->type() == escargot::NodeType::Identifier);
            expr = new escargot::MemberExpressionNode(expr, property, false);
            expr->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
        } else if (ctx->m_lookahead->m_type == Token::TemplateToken && ctx->m_lookahead->m_head) {
            quasi = parseTemplateLiteral(ctx);
            RELEASE_ASSERT_NOT_REACHED();
            // expr = new WrappingNode(startToken).finishTaggedTemplateExpression(expr, quasi);
        } else {
            break;
        }
    }
    return expr;
}

// ECMA-262 12.4 Postfix Expressions

escargot::Node* parsePostfixExpression(ParseContext* ctx)
{
    // var expr, token, startToken = lookahead;
    escargot::Node* expr;

    expr = inheritCoverGrammar(ctx, parseLeftHandSideExpressionAllowCall);

    if (!ctx->m_hasLineTerminator && ctx->m_lookahead->m_type == Token::PunctuatorToken) {
        if (match(ctx, PlusPlus) || match(ctx, MinusMinus)) {
            // ECMA-262 11.3.1, 11.3.2
            if (ctx->m_strict && expr->type() == escargot::NodeType::Identifier && isRestrictedWord(((escargot::IdentifierNode *)expr)->name())) {
                // tolerateError(u"Messages.StrictLHSPostfix");
                throwRestrictedWordUsedError(((escargot::IdentifierNode *)expr)->name());
            }

            if (!ctx->m_isAssignmentTarget) {
                // tolerateError(u"Messages.InvalidLHSInAssignment");
            }

            ctx->m_isAssignmentTarget = ctx->m_isBindingElement = false;

            RefPtr<ParseStatus> token = lex(ctx);
            // expr = new WrappingNode(startToken).finishPostfixExpression(token.value, expr);
            if (token->m_punctuatorsKind == PlusPlus)
                expr = new escargot::UpdateExpressionIncrementPostfixNode(expr);
            else
                expr = new escargot::UpdateExpressionDecrementPostfixNode(expr);
            expr->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
        }
    }

    return expr;
}


// ECMA-262 12.5 Unary Operators

escargot::Node* parseUnaryExpression(ParseContext* ctx)
{
    // var token, expr, startToken;

    RefPtr<ParseStatus> token;
    // RefPtr<ParseStatus> startToken;
    escargot::Node* expr;
    if (ctx->m_lookahead->m_type != Token::PunctuatorToken && ctx->m_lookahead->m_type != Token::KeywordToken) {
        expr = parsePostfixExpression(ctx);
    } else if (match(ctx, PlusPlus) || match(ctx, MinusMinus)) {
        // startToken = ctx->m_lookahead;
        token = lex(ctx);
        expr = inheritCoverGrammar(ctx, parseUnaryExpression);
        // ECMA-262 11.4.4, 11.4.5
        if (ctx->m_strict && expr->type() == escargot::NodeType::Identifier && isRestrictedWord(((escargot::IdentifierNode *)expr)->name())) {
            // tolerateError(u"Messages.StrictLHSPrefix");
            throwRestrictedWordUsedError(((escargot::IdentifierNode *)expr)->name());
        }

        if (!ctx->m_isAssignmentTarget) {
            // tolerateError(u"Messages.InvalidLHSInAssignment");
        }
        // expr = new WrappingNode(startToken).finishUnaryExpression(token.value, expr);
        // expr = new escargot::UnaryExpressionNode(expr, escargot::ESString::create(token->m_value.data()));
        if (token->m_punctuatorsKind == PlusPlus)
            expr = new escargot::UpdateExpressionIncrementPrefixNode(expr);
        else
            expr = new escargot::UpdateExpressionDecrementPrefixNode(expr);
        expr->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
        ctx->m_isAssignmentTarget = ctx->m_isBindingElement = false;
    } else if (match(ctx, Plus) || match(ctx, Minus) || match(ctx, Wave) || match(ctx, ExclamationMark)) {
        // startToken = ctx->m_lookahead;
        token = lex(ctx);
        expr = inheritCoverGrammar(ctx, parseUnaryExpression);
        // expr = new WrappingNode(startToken).finishUnaryExpression(token.value, expr);
        if (token->m_punctuatorsKind == Plus) {
            expr = new escargot::UnaryExpressionPlusNode(expr);
        } else if (token->m_punctuatorsKind == Minus) {
            expr = new escargot::UnaryExpressionMinusNode(expr);
        } else if (token->m_punctuatorsKind == Wave) {
            expr = new escargot::UnaryExpressionBitwiseNotNode(expr);
        } else if (token->m_punctuatorsKind == ExclamationMark) {
            expr = new escargot::UnaryExpressionLogicalNotNode(expr);
        } else {
            RELEASE_ASSERT_NOT_REACHED();
        }
        expr->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
        ctx->m_isAssignmentTarget = ctx->m_isBindingElement = false;
    } else if (matchKeyword(ctx, Delete) || matchKeyword(ctx, Void) || matchKeyword(ctx, Typeof)) {
        // startToken = ctx->m_lookahead;
        token = lex(ctx);
        expr = inheritCoverGrammar(ctx, parseUnaryExpression);
        // expr = new WrappingNode(startToken).finishUnaryExpression(token.value, expr);
        if (token->m_keywordKind == Delete) {
            if (ctx->m_strict && expr->isIdentifier())
                throwEsprimaException(u"Unable to delete variable in strict mode");
            expr = new escargot::UnaryExpressionDeleteNode(expr);
            expr->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
        } else if (token->m_keywordKind == Void) {
            expr = new escargot::UnaryExpressionVoidNode(expr);
            expr->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
        } else if (token->m_keywordKind == Typeof) {
            expr = new escargot::UnaryExpressionTypeOfNode(expr);
            expr->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
        }

        // TODO
        /*
        if (ctx->m_strict && ((escargot::UnaryExpressionNode *)expr)->readOperator() == escargot::UnaryExpressionNode::Operator::Delete
                && ((escargot::UnaryExpressionNode *)expr)->argument()->type() == escargot::NodeType::Identifier) {
            tolerateError(u"Messages.StrictDelete");
        }
        */
        ctx->m_isAssignmentTarget = ctx->m_isBindingElement = false;
    } else {
        expr = parsePostfixExpression(ctx);
    }

    return expr;
}

int binaryPrecedence(ParseContext* ctx, RefPtr<ParseStatus> token, bool allowIn)
{
    if (token->m_type == Token::PunctuatorToken) {
        if (token->m_punctuatorsKind == Substitution) {
            return 0;
        } else if (token->m_punctuatorsKind == LogicalOr) {
            return 1;
        } else if (token->m_punctuatorsKind == LogicalAnd) {
            return 2;
        } else if (token->m_punctuatorsKind == BitwiseOr) {
            return 3;
        } else if (token->m_punctuatorsKind == BitwiseXor) {
            return 4;
        } else if (token->m_punctuatorsKind == BitwiseAnd) {
            return 5;
        } else if (token->m_punctuatorsKind == Equal) {
            return 6;
        } else if (token->m_punctuatorsKind == NotEqual) {
            return 6;
        } else if (token->m_punctuatorsKind == StrictEqual) {
            return 6;
        } else if (token->m_punctuatorsKind == NotStrictEqual) {
            return 6;
        } else if (token->m_punctuatorsKind == RightInequality) {
            return 7;
        } else if (token->m_punctuatorsKind == LeftInequality) {
            return 7;
        } else if (token->m_punctuatorsKind == RightInequalityEqual) {
            return 7;
        } else if (token->m_punctuatorsKind == LeftInequalityEqual) {
            return 7;
        } else if (token->m_punctuatorsKind == LeftShift) {
            return 8;
        } else if (token->m_punctuatorsKind == RightShift) {
            return 8;
        } else if (token->m_punctuatorsKind == UnsignedRightShift) {
            return 8;
        } else if (token->m_punctuatorsKind == Plus) {
            return 9;
        } else if (token->m_punctuatorsKind == Minus) {
            return 9;
        } else if (token->m_punctuatorsKind == Multiply) {
            return 11;
        } else if (token->m_punctuatorsKind == Divide) {
            return 11;
        } else if (token->m_punctuatorsKind == Mod) {
            return 11;
        }
        return 0;
    } else if (token->m_type == Token::KeywordToken) {
        if (token->m_keywordKind == In) {
            return ctx->m_allowIn ? 7 : 0;
        } else if (token->m_keywordKind == InstanceofKeyword) {
            return 7;
        }
    } else {
        return 0;
    }

/*    size_t len = token->m_value.length();
    const char16_t* data = token->m_value.data();
    if (LIKELY(len)) {
        switch (data[0]) {
        case '=':
        {
            if (len == 1) {
                return 0;
            } else if (len == 2) {
                if (data[1] == '=') {
                    return 6;
                }
            } else if (len == 3) {
                if (data[1] == '=' && data[2] == '=') {
                    return 6;
                }
            }
            break;
        }
        case '|':
        {
            if (len == 1) {
                return 3;
            } else if (len == 2 && data[1] == '|') {
                return 1;
            }
            break;
        }
        case '&':
        {
            if (len == 1) {
                return 5;
            } if (len == 2 && data[1] == '&') {
                return 2;
            }
            break;
        }
        case '^':
        {
            if (len == 1) {
                return 4;
            }
            break;
        }
        case '!':
        {
            if (len == 2 && data[1] == '=') {
                return 6;
            } else if (len == 3 && data[1] == '=' && data[2] == '=') {
                return 6;
            }
            break;
        }
        case 'i':
        {
            if (len == 2 && data[1] == 'n') {
                return ctx->m_allowIn ? 7 : 0;
            } else if (token->m_value == u"instanceof") {
                return 7;
            }
            break;
        }
        case '+':
        {
            if (len == 1) {
                return 9;
            }
            break;
        }
        case '-':
        {
            if (len == 1) {
                return 9;
            }
            break;
        }
        case '*':
        {
            if (len == 1) {
                return 11;
            }
            break;
        }
        case '/':
        {
            if (len == 1) {
                return 11;
            }
            break;
        }
        case '%':
        {
            if (len == 1) {
                return 11;
            }
            break;
        }
        case '>':
        {
            if (len == 1) {
                return 7;
            } else if (len == 2 && data[1] == '>') {
                return 8;
            } else if (len == 2 && data[1] == '=') {
                return 7;
            } else if (len == 3 && data[1] == '>' && data[2] == '>') {
                return 8;
            }
            break;
        }
        case '<':
        {
            if (len == 1) {
                return 7;
            } else if (len == 2 && data[1] == '<') {
                return 8;
            } else if (len == 2 && data[1] == '=') {
                return 7;
            }
            break;
        }


        }
    }
    */
    return 0;
}

// ECMA-262 12.6 Multiplicative Operators
// ECMA-262 12.7 Additive Operators
// ECMA-262 12.8 Bitwise Shift Operators
// ECMA-262 12.9 Relational Operators
// ECMA-262 12.10 Equality Operators
// ECMA-262 12.11 Binary Bitwise Operators
// ECMA-262 12.12 Binary Logical Operators

escargot::Node* finishBinaryExpression(ParseContext* ctx, escargot::Node* left, escargot::Node* right, PunctuatorsKind oper)
{
    // Additive Operators
    escargot::Node* nd;
    if (oper == Plus)
        nd = new escargot::BinaryExpressionPlusNode(left, right);
    else if (oper == Minus)
        nd = new escargot::BinaryExpressionMinusNode(left, right);

    // Bitwise Shift Operators
    else if (oper == LeftShift)
        nd = new escargot::BinaryExpressionLeftShiftNode(left, right);
    else if (oper == RightShift)
        nd = new escargot::BinaryExpressionSignedRightShiftNode(left, right);
    else if (oper == UnsignedRightShift)
        nd = new escargot::BinaryExpressionUnsignedRightShiftNode(left, right);

    // Multiplicative Operators
    else if (oper == Multiply)
        nd = new escargot::BinaryExpressionMultiplyNode(left, right);
    else if (oper == Divide)
        nd = new escargot::BinaryExpressionDivisionNode(left, right);
    else if (oper == Mod)
        nd = new escargot::BinaryExpressionModNode(left, right);

    // Relational Operators
    else if (oper == LeftInequality)
        nd = new escargot::BinaryExpressionLessThanNode(left, right);
    else if (oper == RightInequality)
        nd = new escargot::BinaryExpressionGreaterThanNode(left, right);
    else if (oper == LeftInequalityEqual)
        nd = new escargot::BinaryExpressionLessThanOrEqualNode(left, right);
    else if (oper == RightInequalityEqual)
        nd = new escargot::BinaryExpressionGreaterThanOrEqualNode(left, right);

    // Equality Operators
    else if (oper == Equal)
        nd = new escargot::BinaryExpressionEqualNode(left, right);
    else if (oper == NotEqual)
        nd = new escargot::BinaryExpressionNotEqualNode(left, right);
    else if (oper == StrictEqual)
        nd = new escargot::BinaryExpressionStrictEqualNode(left, right);
    else if (oper == NotStrictEqual)
        nd = new escargot::BinaryExpressionNotStrictEqualNode(left, right);

    // Binary Bitwise Operator
    else if (oper == BitwiseAnd)
        nd = new escargot::BinaryExpressionBitwiseAndNode(left, right);
    else if (oper == BitwiseXor)
        nd = new escargot::BinaryExpressionBitwiseXorNode(left, right);
    else if (oper == BitwiseOr)
        nd = new escargot::BinaryExpressionBitwiseOrNode(left, right);
    else if (oper == LogicalOr)
        nd = new escargot::BinaryExpressionLogicalOrNode(left, right);
    else if (oper == LogicalAnd)
        nd = new escargot::BinaryExpressionLogicalAndNode(left, right);
    else if (oper == InPunctuator)
        nd = new escargot::BinaryExpressionInNode(left, right);
    else if (oper == InstanceOfPunctuator)
        nd = new escargot::BinaryExpressionInstanceOfNode(left, right);
    // TODO
    else
        RELEASE_ASSERT_NOT_REACHED();

    nd->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
    return nd;
}

escargot::Node* parseBinaryExpression(ParseContext* ctx)
{
    // var marker, markers, expr, token, prec, stack, right, operator, left, i;

    PunctuatorsKind operator_;
    RefPtr<ParseStatus> marker = ctx->m_lookahead;
    escargot::Node* left = inheritCoverGrammar(ctx, parseUnaryExpression);

    RefPtr<ParseStatus> token = ctx->m_lookahead;
    int prec = binaryPrecedence(ctx, token, ctx->m_allowIn);
    if (prec == 0) {
        return left;
    }
    ctx->m_isAssignmentTarget = ctx->m_isBindingElement = false;
    token->m_prec = prec;
    lex(ctx);

    std::vector<RefPtr<ParseStatus> > markers;
    markers.push_back(marker);
    markers.push_back(ctx->m_lookahead);
    // markers = [marker, lookahead];
    escargot::Node* right = isolateCoverGrammar(ctx, parseUnaryExpression);

    std::vector<RefPtr<ParseStatus> > refRooter;
    std::vector<void *, gc_allocator<void *> > stack;
    // stack = [left, token, right];
    stack.push_back(left);
    stack.push_back(token.get());
    refRooter.push_back(token);
    stack.push_back(right);

    while ((prec = binaryPrecedence(ctx, ctx->m_lookahead, ctx->m_allowIn)) > 0) {

        // Reduce: make a binary expression from the three topmost entries.
        while ((stack.size() > 2) && (prec <= ((ParseStatus *)stack[stack.size() - 2])->m_prec)) {
            right = (escargot::Node *)stack[stack.size()-1];
            stack.pop_back();
            // right = stack.pop_back();
            operator_ = ((ParseStatus*)stack[stack.size()-1])->m_punctuatorsKind;
            stack.pop_back();
            // operator = stack.pop().value;
            left = (escargot::Node *)stack[stack.size()-1];
            stack.pop_back();
            // left = stack.pop();
            markers.pop_back();
            // expr = new WrappingNode(markers[markers.length - 1]).finishBinaryExpression(operator, left, right);
            escargot::Node* expr = finishBinaryExpression(ctx, left, right, operator_);
            stack.push_back(expr);
        }

        // Shift.
        token = lex(ctx);
        token->m_prec = prec;
        stack.push_back(token.get());
        refRooter.push_back(token);
        markers.push_back(ctx->m_lookahead);
        escargot::Node* expr = isolateCoverGrammar(ctx, parseUnaryExpression);
        stack.push_back(expr);
    }

    // Final reduce to clean-up the stack.
    int i = stack.size() - 1;
    escargot::Node* expr = (escargot::Node *)stack[i];
    markers.pop_back();
    while (i > 1) {
        // expr = new WrappingNode(markers.pop()).finishBinaryExpression(, );
        markers.pop_back();
        expr = finishBinaryExpression(ctx, (escargot::Node *)stack[i - 2], expr, ((ParseStatus *)stack[i - 1])->m_punctuatorsKind);
        i -= 2;
    }

    return expr;
}


// ECMA-262 12.13 Conditional Operator

escargot::Node* parseConditionalExpression(ParseContext* ctx)
{
    // var expr, previousAllowIn, consequent, alternate, startToken;

    // RefPtr<ParseStatus> startToken = ctx->m_lookahead;
    escargot::Node* consequent;
    escargot::Node* alternate;
    escargot::Node* expr = inheritCoverGrammar(ctx, parseBinaryExpression);
    if (match(ctx, GuessMark)) {
        lex(ctx);
        bool previousAllowIn = ctx->m_allowIn;
        ctx->m_allowIn = true;
        consequent = isolateCoverGrammar(ctx, parseAssignmentExpression);
        ctx->m_allowIn = previousAllowIn;
        expect(ctx, Colon);
        alternate = isolateCoverGrammar(ctx, parseAssignmentExpression);

        // expr = new WrappingNode(startToken).finishConditionalExpression(expr, consequent, alternate);
        expr = new escargot::ConditionalExpressionNode(expr, consequent, alternate);
        expr->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
        ctx->m_isAssignmentTarget = ctx->m_isBindingElement = false;
    }

    return expr;
}

// ECMA-262 14.4 Yield expression

escargot::Node* parseYieldExpression(ParseContext* ctx)
{
    // todo
    RELEASE_ASSERT_NOT_REACHED();

   /*
    var argument, expr, delegate, previousAllowYield;

    argument = null;
    expr = new Node();

    expectKeyword('yield');

    if (!hasLineTerminator) {
        previousAllowYield = state.allowYield;
        state.allowYield = false;
        delegate = match('*');
        if (delegate) {
            lex();
            argument = parseAssignmentExpression();
        } else {
            if (!match(';') && !match('}') && !match(')') && lookahead.type !== Token.EOF) {
                argument = parseAssignmentExpression();
            }
        }
        state.allowYield = previousAllowYield;
    }

    return expr.finishYieldExpression(argument, delegate);
    */
}

// ECMA-262 12.14 Assignment Operators

escargot::Node* parseAssignmentExpression(ParseContext* ctx)
{
    // var token, expr, right, list, startToken;

    // RefPtr<ParseStatus> startToken = ctx->m_lookahead;
    RefPtr<ParseStatus> token = ctx->m_lookahead;

    if (!ctx->m_allowYield && matchKeyword(ctx, Yield)) {
        return parseYieldExpression(ctx);
    }

    escargot::Node* right;
    escargot::Node* expr = parseConditionalExpression(ctx);
    /*
    if (expr->type === PlaceHolders.ArrowParameterPlaceHolder || match('=>')) {
        isAssignmentTarget = isBindingElement = false;
        list = reinterpretAsCoverFormalsList(expr);

        if (list) {
            firstCoverInitializedNameError = null;
            return parseArrowFunctionExpression(list, new WrappingNode(startToken));
        }

        return expr;
    }
    */

    if (matchAssign(ctx)) {
        if (!ctx->m_isAssignmentTarget) {
            // tolerateError(u"Messages.InvalidLHSInAssignment");
        }

        // ECMA-262 11.13.1
        if (ctx->m_strict && expr->type() == escargot::NodeType::Identifier && isRestrictedWord(((escargot::IdentifierNode *)expr)->name())) {
            // tolerateUnexpectedToken();
            throwRestrictedWordUsedError(((escargot::IdentifierNode *)expr)->name());
        }

        if (!match(ctx, Substitution)) {
            ctx->m_isAssignmentTarget = ctx->m_isBindingElement = false;
        } else {
            reinterpretExpressionAsPattern(ctx, expr);
        }

        token = lex(ctx);
        right = isolateCoverGrammar(ctx, parseAssignmentExpression);
        // expr = new WrappingNode(startToken).finishAssignmentExpression(token.value, expr, right);
        if (token->m_punctuatorsKind == Substitution) {
            expr = new escargot::AssignmentExpressionSimpleNode(expr, right);
        } else {
            if (token->m_punctuatorsKind == PlusEqual) {
                expr = new escargot::AssignmentExpressionPlusNode(expr, right);
            } else if (token->m_punctuatorsKind == MinusEqual) {
                expr = new escargot::AssignmentExpressionMinusNode(expr, right);
            } else if (token->m_punctuatorsKind == MultiplyEqual) {
                expr = new escargot::AssignmentExpressionMultiplyNode(expr, right);
            } else if (token->m_punctuatorsKind == DivideEqual) {
                expr = new escargot::AssignmentExpressionDivisionNode(expr, right);
            } else if (token->m_punctuatorsKind == ModEqual) {
                expr = new escargot::AssignmentExpressionModNode(expr, right);
            } else if (token->m_punctuatorsKind == LeftShiftEqual) {
                expr = new escargot::AssignmentExpressionLeftShiftNode(expr, right);
            } else if (token->m_punctuatorsKind == RightShiftEqual) {
                expr = new escargot::AssignmentExpressionSignedRightShiftNode(expr, right);
            } else if (token->m_punctuatorsKind == UnsignedRightShiftEqual) {
                expr = new escargot::AssignmentExpressionUnsignedShiftNode(expr, right);
            } else if (token->m_punctuatorsKind == BitwiseXorEqual) {
                expr = new escargot::AssignmentExpressionBitwiseXorNode(expr, right);
            } else if (token->m_punctuatorsKind == BitwiseAndEqual) {
                expr = new escargot::AssignmentExpressionBitwiseAndNode(expr, right);
            } else if (token->m_punctuatorsKind == BitwiseOrEqual) {
                expr = new escargot::AssignmentExpressionBitwiseOrNode(expr, right);
            } else {
                RELEASE_ASSERT_NOT_REACHED();
            }
        }
        expr->setSourceLocation(ctx->m_lineNumber, ctx->m_lineStart);
        ctx->m_firstCoverInitializedNameError = NULL;
    }

    return expr;
}

escargot::StatementNodeVector parseScriptBody(ParseContext* ctx)
{
    // var statement, body = [], token, directive, firstRestricted;
    escargot::StatementNodeVector body;
    ctx->m_currentBody = &body;
    RefPtr<ParseStatus> token;
    RefPtr<ParseStatus> firstRestricted;
    while (ctx->m_startIndex < ctx->m_length) {
        token = ctx->m_lookahead;
        if (token->m_type != Token::StringLiteralToken) {
            break;
        }

        bool strict = true;
        if (token->m_end - 1 - (token->m_start + 1) == 10) {
            static const char16_t* s = u"use strict";
            for (size_t i = 0 ; i < 10 ; i ++) {
                if (s[i] != ctx->m_sourceString->charAt(token->m_start + 1 + i)) {
                    strict = false;
                }
            }
        } else {
            strict = false;
        }

        escargot::Node* statement = parseStatementListItem(ctx);
        if (statement)
            body.push_back(statement);
        else
            break;

        if (((escargot::ExpressionStatementNode *) statement)->expression()->type() != escargot::NodeType::Literal) {
            // this is not directive
            break;
        } else {
            if (strict) {
                ctx->m_strict = true;
                if (firstRestricted) {
                    // tolerateUnexpectedToken(firstRestricted, Messages.StrictOctalLiteral);
                    throwOctalLiteralUsedError();
                }
            } else {
                if (!firstRestricted && token->m_octal) {
                    firstRestricted = token;
                }
            }
        }

        /*
        escargot::UTF16String directive = ctx->m_source.substr(token->m_start + 1,
            token->m_end - 1 - (token->m_start + 1));
        // directive = source.slice(token.start + 1, token.end - 1);
        if (directive == u"use strict") {
            ctx->m_strict = true;
            if (firstRestricted) {
                // tolerateUnexpectedToken(firstRestricted, Messages.StrictOctalLiteral);
                tolerateUnexpectedToken();
            }
        } else {
            if (!firstRestricted && token->m_octal) {
                firstRestricted = token;
            }
        }
        */
    }

    while (ctx->m_startIndex < ctx->m_length) {
        escargot::Node* statement = parseStatementListItem(ctx);
        /* istanbul ignore if */
        /*
        if (typeof statement === 'undefined') {
            break;
        }
        */
        if (statement)
            body.push_back(statement);
    }

    rearrangeNode(*ctx->m_currentBody);
    return body;
}

escargot::ProgramNode* parseProgram(ParseContext* ctx)
{
    /*
    var body, node;

    peek();
    node = new Node();

    body = ;
    return node.finishProgram(body);
     */

    peek(ctx);
    escargot::ProgramNode* node = new escargot::ProgramNode(parseScriptBody(ctx), ctx->m_strict);

    return node;
}

escargot::ProgramNode* parseSingleFunction(escargot::ESString* argSource, escargot::ESString* bodySource)
{
    ParseContext bodyCtx(bodySource, false);
    try {
        ParseContext argCtx(argSource, false);
        peek(&argCtx);
        escargot::InternalAtomicStringVector argVec = parseParams(&argCtx);
        if (argCtx.m_lookahead->m_type != Token::EOFToken)
            throw EsprimaError(argCtx.m_lineNumber, escargot::ESString::create("Invalid Function(...) argument source code"));

        peek(&bodyCtx);
        escargot::Node* bodyNode = parseFunctionSourceElements(&bodyCtx);
        if (bodyCtx.m_lookahead->m_type != Token::EOFToken)
            throw EsprimaError(bodyCtx.m_lineNumber, escargot::ESString::create("Invalid Function(...) body source code"));

        escargot::ProgramNode* programNode = new escargot::ProgramNode(makeAnonymousFunctionNodeVector(&bodyCtx, argVec, bodyNode), bodyCtx.m_strict);
        ASSERT(programNode->body()[1]->type() == escargot::NodeType::FunctionDeclaration);
        return programNode;
    } catch(const char16_t* msg) {
        throw EsprimaError(bodyCtx.m_lineNumber, escargot::ESString::create(msg));
    } catch(escargot::ESString* msg) {
        throw EsprimaError(bodyCtx.m_lineNumber, msg);
    } catch(EsprimaError& err) {
        throw err;
    } catch(...) {
        RELEASE_ASSERT_NOT_REACHED();
    }
    RELEASE_ASSERT_NOT_REACHED();
}

escargot::ProgramNode* parse(escargot::ESString* source, bool strict)
{
    ASSERT_STATIC((sizeof(PuncuatorsTokens) / sizeof(char16_t*)) == PunctuatorsKind::PunctuatorsKindEnd,
        "Sizeof punctuators names array should be equal to sizeof punctuators enum");
    ASSERT_STATIC((sizeof(KeywordTokens) / sizeof(char16_t*)) == KeywordKind::KeywordKindEnd,
        "Sizeof keyword names array should be equal to sizeof keyword enum");

    ParseContext ctx(source, strict);
    try {
        return parseProgram(&ctx);
    } catch(const char16_t* msg) {
        throw EsprimaError(ctx.m_lineNumber, escargot::ESString::create(msg));
    } catch(escargot::ESString* msg) {
        throw EsprimaError(ctx.m_lineNumber, msg);
    } catch(EsprimaError& err) {
        throw err;
    } catch(...) {
        RELEASE_ASSERT_NOT_REACHED();
    }
    RELEASE_ASSERT_NOT_REACHED();
}

}
#pragma GCC diagnostic pop // -Wunused-variable
