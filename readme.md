# ghex

A lightweight, terminal-based hexadecimal viewer and search tool written in C99. `ghex` allows you to inspect binary and text files with customizable formatting, precise byte range selection, and pattern searching capabilities.

---

## Features

* **Configurable Layout**: Adjust row byte widths to fit terminal displays.
* **Side-by-Side Views**: View byte offset, hexadecimal byte values, and sanitized ASCII character representations simultaneously.
* **ASCII Suppression**: Toggle ASCII rendering on or off for clean numeric-only outputs.
* **Range Slicing**: Target exact sections of a file using start/end offsets and row count constraints.
* **Pattern Searching**: Scan files for plain-text strings or raw hexadecimal byte sequences and preview matches in place.
* **Multiple File Handling**: Process multiple files in a single invocation.

---

## Installation and Build

### Prerequisites

* GCC or Clang compiler
* GNU Make

### Clone and Compile

1. Clone the repository:
```
bash
git clone [https://github.com/your-username/ghex.git](https://github.com/your-username/ghex.git)
cd ghex
```

2. Compile using `make`:
```
make
```

#### Remove compiled binary:
```
make clean
```

#### Alternatively, compile using gcc:
```
gcc -Wall -Wextra -std=c99 ghex.c -o ghex
```

---

## Usage and Options

```
./ghex [options] <file...>
```

Command-Line Arguments
- `--width=<N>`: Number of bytes displayed per row (default: 16, maximum: 256).

- `--start=<offset>`: Starting byte offset (decimal).

- `--end=<offset>`: Ending byte offset (inclusive, decimal).

- `--rows=<N>`: Maximum number of rows to print.

- `--no-ascii`: Suppress the printable ASCII text column.

- `--search-text="<text>"`: Search for an ASCII text pattern.

- `--search-hex="<hex>"`: Search for a raw hex sequence (space-separated or contiguous).

### Examples

1. Default inspection:
```
./ghex sample.bin
```

2. Custom width
```
./ghex --width=8 sample.bin
```

3. Range-Limited Dump
```
./ghex --start=256 --end=512 sample.bin
```

4. Row-Limited Dump
```
./ghex --start=1024 --rows=4 sample.bin
```

5. Hex-Only View (No ASCII)
```
./ghex --width=16 --no-ascii sample.bin
```

6. Search for Plain Text
```
./ghex --search-text="ELF" sample.bin
```

7. Search for Hex Byte Sequences
```
./ghex --search-hex="7F 45 4C 46" sample.bin
# Or contiguous:
./ghex --search-hex="7f454c46" sample.bin
```

8. Combined Search and Formatting
```
./ghex --search-hex="89 50 4E 47" --rows=2 --no-ascii image.png
```

---

## Upcoming Features
- Interactive Mode: A terminal-based navigation interface (TUI) allowing real-time scrolling and arbitrary jumping across large binaries.
- Editing Mode: In-place byte and ASCII editing capabilities directly within the terminal interface, including diff previews before saving changes.

