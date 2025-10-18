/**
 * @file main.cpp
 * @brief PAQMan - A high-performance file compression and decompression tool using libzpaq.
 *
 * PAQMan is a command-line utility for compressing and decompressing files using the ZPAQ format.
 * It supports various compression levels (0-5, where 0 is no compression and 5 is the highest).
 *
 * Usage:
 *   paqman c <input_file> <output_file> [method]  # Compress (method: 0-5, default 5)
 *   paqman d <input_file> <output_file>           # Decompress
 *   paqman --help                                 # Show help
 *
 * Examples:
 *   paqman c input.txt compressed.zpaq 3          # Compress with level 3
 *   paqman d compressed.zpaq output.txt           # Decompress
 *
 * Features:
 * - Supports binary and text files.
 * - Error handling for file I/O and libzpaq operations.
 * - Verbose output for progress indication.
 * - MIT Licensed.
 *
 * Dependencies:
 * - libzpaq (included in src/libzpaq.cpp and src/libzpaq.h)
 *
 * Compilation:
 *   clang++ src/libzpaq.cpp src/main.cpp -O2 -lpthread -o paqman
 *
 * Author: Gage
 * Version: 1.0
 * Date: 2025
 */

#include "libzpaq.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>

// Error handling function required by libzpaq
namespace libzpaq {
    void error(const char* msg) {
        std::fprintf(stderr, "libzpaq Error: %s\n", msg);
        std::exit(1);
    }
}

// --- File Reader ---
// Custom Reader implementation for reading from files in binary mode.
class FileReader : public libzpaq::Reader {
private:
    std::ifstream in;

public:
    explicit FileReader(const std::string& filename) {
        in.open(filename, std::ios::binary);
        if (!in.is_open()) {
            throw std::runtime_error("Cannot open input file: " + filename);
        }
    }

    ~FileReader() override {
        if (in.is_open()) {
            in.close();
        }
    }

    int get() override {
        if (in.eof()) {
            return -1;  // EOF
        }
        return in.get();
    }

    // Override for efficient block reads (optional optimization)
    int read(char* buf, int n) override {
        in.read(buf, n);
        return static_cast<int>(in.gcount());
    }
};

// --- File Writer ---
// Custom Writer implementation for writing to files in binary mode.
class FileWriter : public libzpaq::Writer {
private:
    std::ofstream out;

public:
    explicit FileWriter(const std::string& filename) {
        out.open(filename, std::ios::binary);
        if (!out.is_open()) {
            throw std::runtime_error("Cannot open output file: " + filename);
        }
    }

    ~FileWriter() override {
        if (out.is_open()) {
            out.flush();
            out.close();
        }
    }

    void put(int c) override {
        out.put(static_cast<char>(c));
    }

    // Override for efficient block writes (optional optimization)
    void write(const char* buf, int n) override {
        out.write(buf, n);
    }
};

// --- Compression ---
// Compresses the input file to the output file using the specified ZPAQ method.
// Method: "0" to "5" (0=store, 5=best compression).
void compressFile(const std::string& input, const std::string& output, const std::string& method = "5") {
    std::cout << "Compressing: " << input << " -> " << output << " (method: " << method << ")\n";

    FileReader in(input);
    FileWriter out(output);

    // Use libzpaq to compress
    libzpaq::compress(&in, &out, method.c_str(), input.c_str(), nullptr, true);  // Include filename in archive

    std::cout << "Compression complete: " << output << "\n";
}

// --- Decompression ---
// Decompresses the input ZPAQ file to the output file.
void decompressFile(const std::string& input, const std::string& output) {
    std::cout << "Decompressing: " << input << " -> " << output << "\n";

    FileReader in(input);
    FileWriter out(output);

    // Use libzpaq to decompress
    libzpaq::decompress(&in, &out);

    std::cout << "Decompression complete: " << output << "\n";
}

// --- Main ---
// Entry point for the PAQMan application.
// Parses command-line arguments and dispatches to compression or decompression.
int main(int argc, char** argv) {
    if (argc < 2 || std::string(argv[1]) == "--help") {
        std::cout << "PAQMan v1.0 - Ultra Compression Tool using ZPAQ\n\n";
        std::cout << "Usage:\n";
        std::cout << "  paqman c <input_file> <output_file> [method]  # Compress (method: 0-5, default 5)\n";
        std::cout << "  paqman d <input_file> <output_file>           # Decompress\n";
        std::cout << "  paqman --help                                # Show this help\n\n";
        std::cout << "Methods:\n";
        std::cout << "  0: Store only (no compression)\n";
        std::cout << "  1-5: Increasing compression levels (5 is slowest/best)\n\n";
        std::cout << "Examples:\n";
        std::cout << "  paqman c input.txt compressed.zpaq 3\n";
        std::cout << "  paqman d compressed.zpaq output.txt\n\n";
        std::cout << "For more details, see the file header or LICENSE.\n";
        return 0;
    }

    if (argc < 4) {
        std::cerr << "Error: Insufficient arguments. Use --help for usage.\n";
        return 1;
    }

    std::string mode = argv[1];
    std::string input = argv[2];
    std::string output = argv[3];

    // Validate input file exists
    std::ifstream check(input);
    if (!check.good()) {
        std::cerr << "Error: Input file '" << input << "' does not exist or is inaccessible.\n";
        return 1;
    }
    check.close();

    try {
        if (mode == "c") {
            std::string method = (argc > 4) ? std::string(argv[4]) : "5";
            // Basic validation for method (0-5)
            if (method.length() != 1 || method < "0" || method > "5") {
                std::cerr << "Error: Invalid method '" << method << "'. Use 0-5.\n";
                return 1;
            }
            compressFile(input, output, method);
        } else if (mode == "d") {
            decompressFile(input, output);
        } else {
            std::cerr << "Error: Unknown mode '" << mode << "'. Use 'c' or 'd'.\n";
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred.\n";
        return 1;
    }

    return 0;
}
