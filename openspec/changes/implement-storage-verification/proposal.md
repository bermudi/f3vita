# Change: Implement f3vita Storage Verification Tool

## Why
Users need a reliable way to verify PS Vita storage integrity, especially for SD2Vita adapters and potentially counterfeit SD cards. No equivalent tool exists for the Vita platform.

## What Changes
- Create new PS Vita homebrew application "f3vita"
- Implement storage selection menu with detected mount points
- Implement write phase with deterministic test patterns
- Implement verify phase with byte-level comparison
- Add real-time progress display using debug screen
- Support user cancellation and cleanup

## Impact
- Affected specs: `specs/storage-verification/spec.md` (new)
- Affected code: New project (no existing code)
- New files:
  - `CMakeLists.txt` - Build configuration
  - `src/main.c` - Entry point, state machine, UI loop
  - `src/storage.c` - Storage enumeration, file I/O
  - `src/pattern.c` - Pattern generation and verification
  - `src/ui.c` - Debug screen display helpers
  - `include/*.h` - Header files
  - `sce_sys/` - Vita app metadata