# f3vita

**Storage Verification Tool for PS Vita**

Verify storage integrity on PS Vita by writing known patterns and reading them back. Detects bad sectors, fake capacity, or flaky adapters (especially useful for SD2Vita).

Named after the popular [f3](https://fight-flash-fraud.readthedocs.io/) tool for desktop systems.

## Features

- **Storage Selection**: Interactive menu to select target storage (ux0:, uma0:, etc.)
- **Write Phase**: Writes 1GB test files with deterministic patterns
- **Verify Phase**: Reads back and compares patterns to detect corruption
- **Real-time Progress**: Shows MB processed, percentage, elapsed time, and speed
- **Error Reporting**: Reports total corrupted bytes and first bad block location
- **Cleanup Option**: Optionally deletes test files after completion

## Building

### Prerequisites

- [VitaSDK](https://vitasdk.org/) installed and configured
- CMake 3.10+

### Build Steps

```bash
# Set VitaSDK environment
export VITASDK=/path/to/vitasdk
export PATH=$VITASDK/bin:$PATH

# Generate assets (first time only)
pip install pillow
python generate_assets.py

# Build
mkdir -p build && cd build
cmake ..
make
```

The output `f3vita.vpk` will be in the `build/` directory.

## Installation

1. Copy `f3vita.vpk` to your PS Vita
2. Install using VitaShell or similar
3. Launch from LiveArea

## Usage

1. **Select Storage**: Use D-pad to select target storage, press X to start
2. **Write Phase**: Tool writes test files until disk is full
3. **Verify Phase**: Tool reads back and verifies all patterns
4. **Results**: View pass/fail status and corruption summary
5. **Cleanup**: Choose to delete test files or keep them

### Controls

| Button | Action |
|--------|--------|
| D-Pad Up/Down | Navigate menu |
| X | Confirm / Start test |
| O | Cancel / Exit |

## How It Works

1. Creates test directory: `<target>/data/f3vita/`
2. Writes 1GB files named `f3vita_001.dat`, `f3vita_002.dat`, etc.
3. Each 1MB block contains a deterministic pattern based on file/block index
4. Reads back all data and compares against expected pattern
5. Reports any mismatches as corruption

### Test Pattern

Each byte's expected value is: `(file_index << 24) ^ (block_index << 16) ^ byte_offset`

This ensures:
- Every location has a unique expected value
- Address aliasing (fake capacity) is detected
- Bit-flip corruption is detected

## Supported Devices

- PS Vita (PCH-1000, PCH-2000)
- PS TV (VTE-1000)

## Supported Storage

- `ux0:` - Memory Card / Internal storage
- `uma0:` - USB storage (PSTV only)
- `imc0:` - Internal memory (Slim only)

## License

MIT License

## Acknowledgments

- [VitaSDK](https://vitasdk.org/) team
- [f3](https://fight-flash-fraud.readthedocs.io/) for the original concept