/**
 * @file main.cpp
 * @brief PAQMan - A high-performance file compression and decompression tool using libzpaq.
 *
 * PAQMan is a command-line utility for compressing and decompressing files using the ZPAQ format.
 * It supports various compression levels (0-5, where 0 is no compression and 5 is the highest).
 *
 * Usage:
 *   paqman c <input_file_or_dir> <output_file> [method]  # Compress file or directory (method: 0-5, default 5)
 *   paqman d <input_file> <output_dir>                  # Decompress to directory
 *   paqman l <input_file>                               # List contents of archive
 *   paqman --help                                       # Show help
 *
 * Examples:
 *   paqman c input.txt compressed.zpaq 3          # Compress with level 3
 *   paqman c mydir archive.zpaq 5                 # Compress directory
 *   paqman d archive.zpaq output_dir              # Decompress to directory
 *   paqman l archive.zpaq                         # List archive contents
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
 *  __ _ __ _  __ _ ____ _ ____  ___ ______
 * |  | '_ \ / _` |/ __| '_ ` _ \ / _` | '_\
 * | |_) | (_| | (__| | | | | | (_| | | | |
 * | .__/ \__,_|\___|_| |_| |_|\__,_|_| |_|
 * |_|              |_|
 *                  \__/
 *
 * Maintainer: Gage
 * Version: 1.0.1
 * Date: 2025-10-17 20:01
 */

#include "libzpaq.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <filesystem>

namespace fs = std::filesystem;

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

// --- Null Writer ---
// Custom Writer implementation that discards all data (used for listing without extraction).
class NullWriter : public libzpaq::Writer {
public:
    void put(int c) override {
        // Discard the byte
    }

    void write(const char* buf, int n) override {
        // Discard the buffer
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

// --- Compress Directory ---
// Compresses all files in the input directory recursively to the output file.
void compressDirectory(const std::string& inputDir, const std::string& output, const std::string& method = "5") {
    std::cout << "Compressing directory: " << inputDir << " -> " << output << " (method: " << method << ")\n";

    libzpaq::Compressor c;
    c.setOutput(new FileWriter(output));
    c.startBlock(atoi(method.c_str()));  // Use level based on method

    // Recursively add files
    for (const auto& entry : fs::recursive_directory_iterator(inputDir)) {
        if (entry.is_regular_file()) {
            std::string relativePath = fs::relative(entry.path(), inputDir).string();
            std::cout << "Adding: " << relativePath << "\n";

            c.startSegment(relativePath.c_str());
            FileReader in(entry.path().string());
            c.setInput(&in);
            while (c.compress(1000000));  // Compress in chunks
            c.endSegment();
        }
    }

    c.endBlock();
    std::cout << "Directory compression complete: " << output << "\n";
}

// --- Decompress to Directory ---
// Decompresses the input ZPAQ file to the output directory, preserving structure.
void decompressToDirectory(const std::string& input, const std::string& outputDir) {
    std::cout << "Decompressing: " << input << " -> " << outputDir << "\n";

    // Create output directory if it doesn't exist
    fs::create_directories(outputDir);

    FileReader in(input);
    libzpaq::Decompresser d;
    d.setInput(&in);

    double memory = 0;
    if (!d.findBlock(&memory)) {
        throw std::runtime_error("\33[31mNo valid ZPAQ block found in " + input + "\33[0m");
    }

    while (d.findFilename()) {
        // Get filename
        std::string filename;
        {
            char buf[4096];
            int len = 0;
            while ((buf[len] = d.get()) != -1 && len < 4095) ++len;
            buf[len] = 0;
            filename = buf;
        }

        // Create full output path
        fs::path outPath = fs::path(outputDir) / filename;
        fs::create_directories(outPath.parent_path());

        // Set output to file
        FileWriter out(outPath.string());
        d.setOutput(&out);

        // Decompress segment
        while (d.decompress(1000000));

        // Read segment end
        char sha1[21];
        d.readSegmentEnd(sha1);

        std::cout << "Extracted: " << filename << "\n";
    }

    std::cout << "Directory decompression complete: " << outputDir << "\n";
}

// --- List Archive Contents ---
// Lists the contents of the input ZPAQ file without extracting.
void listArchiveContents(const std::string& input) {
    std::cout << "Listing contents of: " << input << "\n";

    FileReader in(input);
    libzpaq::Decompresser d;
    d.setInput(&in);

    double memory = 0;
    if (!d.findBlock(&memory)) {
        throw std::runtime_error("\33[31mNo valid ZPAQ block found in " + input + "\33[0m");
    }

    NullWriter nullOut;
    d.setOutput(&nullOut);

    while (d.findFilename()) {
        // Get filename
        std::string filename;
        {
            char buf[4096];
            int len = 0;
            while ((buf[len] = d.get()) != -1 && len < 4095) ++len;
            buf[len] = 0;
            filename = buf;
        }

        std::cout << filename << "\n";

        // Skip decompression by decompressing to null writer
        while (d.decompress(1000000));

        // Read segment end
        char sha1[21];
        d.readSegmentEnd(sha1);
    }

    std::cout << "Listing complete.\n";
}

// --- Main ---
// Entry point for the PAQMan application.
// Parses command-line arguments and dispatches to compression or decompression.
int main(int argc, char** argv) {
    if (argc < 2 || std::string(argv[1]) == "--help") {
        std::cout << "-(\33[31mPAQMan\33[0m)-By-(\33[31mGage\33[0m)-\n\n";
        std::cout << "Usage:\n";
        std::cout << "  \33[31mpaqman c <input_file_or_dir> <output_file> [method]\33[0m  # Compress file or directory (method: 0-5, default 5)\n";
        std::cout << "  \33[31mpaqman d <input_file> <output_dir>\33[0m                  # Decompress to directory\n";
        std::cout << "  \33[31mpaqman --help\33[0m                                       # Show this help\n";
        std::cout << "  \33[31mpaqman l <archive>\33[0m				    # list files in the compressed archive\n\n";
        std::cout << "Methods:\n";
        std::cout << "  \33[31m0\33[0m: Store only (no compression)\n";
        std::cout << "  \33[31m1-5\33[0m: Increasing compression levels (5 is slowest/best)\n\n";
        std::cout << "Examples:\n";
        std::cout << "  \33[31mpaqman c input.txt compressed.zpaq 3\33[0m\n";
        std::cout << "  \33[31mpaqman c mydir archive.zpaq 5\33[0m\n";
        std::cout << "  \33[31mpaqman d compressed.zpaq output_dir\33[0m\n\n";
        std::cout << "For more details, see the file header or LICENSE.\n";
        return 0;
    }

    if (argc < 4) {
        std::cerr << "\33[31mError: Insufficient arguments. Use --help for usage.\33[0m\n";
        return 1;
    }

    std::string mode = argv[1];
    std::string input = argv[2];
    std::string output = argv[3];

    // Validate input file exists
    std::ifstream check(input);
    if (!check.good()) {
        std::cerr << "\33[31mError: Input file '" << input << "' mdoes not exist or is inaccessible.\33[0m\n";
        return 1;
    }
    check.close();

    try {
        if (mode == "c") {
            std::string method = (argc > 4) ? std::string(argv[4]) : "5";
            // Basic validation for method (0-5)
            if (method.length() != 1 || method < "0" || method > "5") {
                std::cerr << "\33[31mError: Invalid method '" << method << "'. Use 0-5.\33[0m\n";
                return 1;
            }
            if (fs::is_directory(input)) {
                compressDirectory(input, output, method);
            } else {
                compressFile(input, output, method);
            }
        } else if (mode == "d") {
            decompressToDirectory(input, output);
        } else {
            std::cerr << "\33[31mError: Unknown mode '" << mode << "'. Use 'c', 'd', or 'l'.\33[0m\n";
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "\33[31mError: \33[0m" << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "\33[31mUnknown error occurred.\33[0m\n";
        return 1;
    }

    return 0;
}
