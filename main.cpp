#include "libzpaq.h"
#include <iostream>
#include <fstream>
#include <string>
  
// --- File Reader ---
class FileReader : public libzpaq::Reader {
    std::ifstream in;
public:
    explicit FileReader(const std::string& filename) {
        in.open(filename, std::ios::binary);
        if (!in) throw std::runtime_error("Cannot open input file: " + filename);
    }
    int get() override {
        return in.get();
    }
    bool eof() {  // no override — not virtual in libzpaq
        return in.eof();
    }
};

// --- File Writer ---
class FileWriter : public libzpaq::Writer {
    std::ofstream out;
public:
    explicit FileWriter(const std::string& filename) {
        out.open(filename, std::ios::binary);
        if (!out) throw std::runtime_error("Cannot open output file: " + filename);
    }
    void put(int c) override {
        out.put(c);
    }
    void flush() {  // no override — not virtual in libzpaq
        out.flush();
    }
};

// --- Compression ---
void compressFile(const std::string& input, const std::string& output, const std::string& method = "5") {
    FileReader in(input);
    FileWriter out(output);
    libzpaq::compress(&in, &out, method.c_str(), nullptr);
    std::cout << "Compression complete: " << output << "\n";
}

// --- Decompression ---
void decompressFile(const std::string& input, const std::string& output) {
    FileReader in(input);
    FileWriter out(output);
    libzpaq::decompress(&in, &out);
    std::cout << "Decompression complete: " << output << "\n";
}

// --- Main ---
int main(int argc, char** argv) {
    if (argc < 4) {
        std::cout << "PAQMan - Ultra Compression Tool\n";
        std::cout << "Usage:\n";
        std::cout << "  paqman c <input> <output> [method]\n";
        std::cout << "  paqman d <input> <output>\n";
        return 0;
    }

    std::string mode = argv[1];
    std::string input = argv[2];
    std::string output = argv[3];

    try {
        if (mode == "c") {
            std::string method = (argc > 4) ? argv[4] : "5";
            compressFile(input, output, method);
        } else if (mode == "d") {
            decompressFile(input, output);
        } else {
            std::cerr << "Unknown mode: " << mode << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
