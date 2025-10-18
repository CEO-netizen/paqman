# PAQMan

PAQMan is a high-performance file compression and decompression tool using the ZPAQ format. It provides ultra-efficient compression with support for various levels, making it ideal for archiving, backups, and data storage where size matters.

## Features

- **Ultra Compression**: Based on ZPAQ, one of the most efficient compression algorithms available.
- **Multiple Levels**: Supports compression levels 0-5 (0 = store only, 5 = best compression).
- **Fast Decompression**: Decompresses quickly regardless of compression level.
- **Command-Line Interface**: Simple and intuitive CLI for easy integration into scripts.
- **Error Handling**: Robust error handling for file I/O and compression operations.
- **MIT Licensed**: Free to use, modify, and distribute.

## Installation

### Prerequisites
- C++ compiler (e.g., clang++ or g++)
- Linux or compatible Unix-like system

### Build
```bash
clang++ src/libzpaq.cpp src/main.cpp -O2 -lpthread -o paqman
```

This will create the `paqman` executable.

## Usage

### Compress a File
```bash
paqman c <input_file> <output_file> [method]
```
- `method`: Compression level (0-5, default 5). Higher levels compress better but slower.

Example:
```bash
paqman c input.txt compressed.zpaq 3
```

### Decompress a File
```bash
paqman d <input_file> <output_file>
```

Example:
```bash
paqman d compressed.zpaq output.txt
```

### Help
```bash
paqman --help
```

## Compression Methods

- **0**: Store only (no compression) - Fastest, largest output.
- **1**: Basic compression - Good balance of speed and size.
- **2**: Improved compression - Slower but smaller.
- **3**: High compression - Even better ratio.
- **4**: Very high compression - Slower still.
- **5**: Maximum compression - Slowest but best ratio.

## Examples

### Compress a text file with maximum compression
```bash
paqman c large_file.txt archive.zpaq 5
```

### Compress a binary file with fast compression
```bash
paqman c image.jpg compressed.zpaq 1
```

### Decompress an archive
```bash
paqman d archive.zpaq restored_file.txt
```

## Technical Details

- **Algorithm**: ZPAQ (context mixing with arithmetic coding)
- **Format**: ZPAQ Level 2 format
- **Memory Usage**: Varies by compression level (typically 16MB - 64MB)
- **Threading**: Supports multi-threading for better performance on multi-core systems

## Limitations

- Not suitable for real-time compression (slow for large files at high levels)
- Requires significant memory for high compression levels
- Single-file archives (no multi-file support in this version)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Acknowledgments

- Based on libzpaq by Matt Mahoney
- ZPAQ algorithm developed by Matt Mahoney

## Version

Current version: 1.0
