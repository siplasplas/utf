#include <iostream>
#include <fstream>
#include <utility>
#include <vector>
#include "Args.h"
#include "utf/UTF.hpp"

using namespace std;

void printHelp() {
    cout << "convutf options input_file output_file" << endl;
    cout << "where options is form -cMN, M,N = digits" << endl;
    cout << "for example: convutf -c12 utf8.txt utf16.txt" << endl;
    cout << "available formats: " << endl;
    cout << "1: utf8" << endl;
    cout << "2: utf16" << endl;
    cout << "3: utf16be" << endl;
    cout << "4: utf12" << endl;
    cout << "5: utf32be" << endl;
    cout << "where 'be' means big endian" << endl;
}

u32string readTo32(string inFile, int inFormat) {
    std::ifstream file(inFile, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    UTF utf;
    switch (inFormat) {
        case 1: {
            std::vector<char> buffer(size);
            if (!file.read(buffer.data(), size)) {
                cout << "error reading " << inFile << endl;
                exit(1);
            }
            return utf.toUTF32(string_view(buffer.data(), size));
        }
            break;
        case 2:
        case 3: {
            std::vector<char16_t> buffer(size / 2);
            if (!file.read((char *) buffer.data(), buffer.size() * 2)) {
                cout << "error reading " << inFile << endl;
                exit(1);
            }
            if (inFormat == 3)
                for (char16_t &c : buffer)
                    c = UTF::swap16(c);
            return utf.toUTF32(u16string_view(buffer.data(), buffer.size()));
        }
            break;
        case 4:
        case 5: {
            std::vector<char32_t> buffer(size / 4);
            if (!file.read((char *) buffer.data(), buffer.size() * 4)) {
                cout << "error reading " << inFile << endl;
                exit(1);
            }
            if (inFormat == 5)
                for (char32_t &c : buffer)
                    c = UTF::reverse32(c);
            u32string result;
            result = u32string_view(buffer.data(), buffer.size());
            return result;
        }
            break;
    }
    return u32string{};
}

void saveTo(const u32string &u32, const string &outFile, int outFormat) {
    std::ofstream file(outFile, std::ios::binary);
    UTF utf;
    switch (outFormat) {
        case 1: {
            string u8 = utf.fromUTF32(u32);
            file.write(u8.c_str(), u8.size());
        }
            break;
        case 2:
        case 3: {
            u16string u16 = utf.fromUTF32to16(u32);
            if (outFormat == 3)
                utf.swapIt(u16);
            file.write((char *) (u16.c_str()), u16.size() * 2);
        }
            break;
        case 4:
            file.write((char *) (u32.c_str()), u32.size() * 4);
            break;
        case 5: {
            u32string reversed = u32;
            utf.reverseIt(reversed);
            file.write((char *) (reversed.c_str()), reversed.size() * 4);
        }
            break;
        default:;
    }
}

void convert(const string &inFile, const string &outFile, int inFormat, int outFormat) {
    u32string u32 = readTo32(inFile, inFormat);
    saveTo(u32, outFile, outFormat);
}

int main(int argc, char *argv[]) {
    Args args(argc, argv);
    if (argc < 2)
        printHelp();
    else {
        args.parse();
        if (args.error())
            printHelp();
        else convert(args.inFile, args.outFile, args.inFormat, args.outFormat);
    }
}
