# Design: f3vita Storage Verification

## Context
Building a homebrew storage verification tool for PS Vita. Must be simple, reliable, and work with VitaSDK's standard libraries. Users are typically checking SD2Vita adapters with potentially counterfeit or failing SD cards.

## Goals / Non-Goals

**Goals:**
- Detect storage corruption reliably
- Detect fake capacity (write succeeds but data is lost/wrong)
- Simple text-based UI that works on all Vita models
- Single .vpk for easy distribution

**Non-Goals:**
- Speed benchmarking
- SMART data or hardware diagnostics
- Filesystem repair or block remapping
- Fancy graphics or animations

## Technical Decisions

### Decision: State Machine Architecture
Main loop uses a simple state machine pattern.

```c
typedef enum {
    STATE_MENU,      // Storage selection
    STATE_WRITE,     // Writing test files
    STATE_VERIFY,    // Reading and verifying
    STATE_RESULTS,   // Showing summary
    STATE_CLEANUP,   // Deleting files
    STATE_EXIT       // Clean exit
} AppState;

while (state != STATE_EXIT) {
    handle_input(&state, &ctx);
    update_state(&state, &ctx);
    render_state(state, &ctx);
    sceDisplayWaitVblankStart();
}
```

**Why:** Simple, easy to debug, clear flow. No need for threading complexity.

### Decision: 1MB Block Size
All I/O operations use 1MB (1048576 bytes) blocks.

**Why:**
- Large enough for efficient I/O
- Small enough to fit in stack/heap comfortably
- Easy math for progress calculation
- Matches common SD card erase block sizes

### Decision: Test Pattern Formula
```c
// For each byte in a 1MB block:
// pattern = (file_index << 24) ^ (block_index << 16) ^ (byte_offset & 0xFFFF)
void fill_pattern_block(uint8_t *buf, uint32_t file_idx, uint32_t block_idx) {
    uint32_t base = (file_idx << 24) ^ (block_idx << 16);
    for (uint32_t i = 0; i < BLOCK_SIZE; i++) {
        buf[i] = (base ^ i) & 0xFF;
    }
}
```

**Why:**
- Deterministic: same input always produces same output
- Unique per location: catches address aliasing (fake capacity)
- Fast: simple XOR operations
- Verifiable: can regenerate expected value without storing it

### Decision: Multiple 1GB Files
Write multiple files up to 1GB each rather than one giant file.

**Why:**
- FAT32 compatibility (though Vita FS is different, staying safe)
- Easier progress tracking
- Partial test possible (verify files 1-5 even if 6-10 have errors)
- Original f3 does this, proven approach

### Decision: Debug Screen for UI
Use `psvDebugScreenPrintf()` for all display output.

**Why:**
- Zero dependencies beyond VitaSDK
- Simple text output sufficient for this tool
- Works reliably on all models
- Fastest path to working software

## Data Structures

```c
// Storage info for menu
typedef struct {
    char path[16];        // "ux0:", "uma0:", etc.
    char name[32];        // "Memory Card", "USB", etc.
    uint64_t total_bytes;
    uint64_t free_bytes;
    int writable;
} StorageDevice;

// Test context
typedef struct {
    StorageDevice target;
    
    // Write phase tracking
    uint32_t files_written;
    uint64_t bytes_written;
    
    // Verify phase tracking
    uint32_t current_file;
    uint32_t current_block;
    uint64_t bytes_verified;
    uint64_t bytes_corrupted;
    
    // First error location
    int has_first_error;
    uint32_t first_error_file;
    uint32_t first_error_block;
    uint32_t first_error_offset;
    
    // Timing
    uint64_t start_time;
    uint64_t end_time;
    
    // User preference
    int cleanup_requested;
} TestContext;
```

## File Layout

```
<target>/data/f3vita/
├── f3vita_001.dat    # First 1GB test file
├── f3vita_002.dat    # Second 1GB test file
├── ...
└── f3vita_NNN.dat    # Last file (may be < 1GB)
```

## Error Handling

| Error | Handling |
|-------|----------|
| Write fails mid-file | Close file, proceed to verify |
| Disk full | Expected! Close file, proceed to verify |
| Read fails | Count entire block as corrupted, continue |
| Directory creation fails | Show error, return to menu |
| No writable storage | Show message, allow exit only |

## Storage Detection

Use `sceIoDopen()` to check if path exists and is accessible:
```c
const char *candidates[] = {"ux0:", "uma0:", "imc0:", "xmc0:", NULL};
for (int i = 0; candidates[i]; i++) {
    SceUID dir = sceIoDopen(candidates[i]);
    if (dir >= 0) {
        sceIoDclose(dir);
        // Add to list
    }
}
```

## Risks / Trade-offs

| Risk | Mitigation |
|------|------------|
| Very slow on large cards | Progress display keeps user informed; allow cancel |
| Debug screen hard to read | Use spacing, clear labels, colors if available |
| User removes card mid-test | OS will return errors; we handle gracefully |

## Open Questions
- Should we add a "quick test" mode that only tests first/last 1GB? (Deferred to v2)
- Should we show write/verify speed? (Nice to have, low priority)