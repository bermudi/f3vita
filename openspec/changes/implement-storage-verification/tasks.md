# Implementation Tasks

## 1. Project Setup
- [x] 1.1 Create CMakeLists.txt with VitaSDK configuration
- [x] 1.2 Create directory structure (src/, include/, sce_sys/)
- [x] 1.3 Create app metadata (icon0.png placeholder, livearea template)
- [x] 1.4 Verify build produces .vpk file

## 2. Core Infrastructure
- [x] 2.1 Create main.c with entry point and state machine skeleton
- [x] 2.2 Define app states enum (MENU, WRITE, VERIFY, RESULTS, CLEANUP)
- [x] 2.3 Initialize debug screen and input handling
- [x] 2.4 Implement main loop with state dispatch

## 3. Storage Module
- [x] 3.1 Create storage.h/storage.c
- [x] 3.2 Implement storage enumeration (detect ux0:, uma0:, etc.)
- [x] 3.3 Implement get_storage_info() for free/total space
- [x] 3.4 Implement create_test_directory()
- [x] 3.5 Implement file write helper (handles errors gracefully)
- [x] 3.6 Implement file read helper
- [x] 3.7 Implement cleanup_test_files()

## 4. Pattern Module
- [x] 4.1 Create pattern.h/pattern.c
- [x] 4.2 Implement fill_pattern_block(buffer, file_idx, block_idx)
- [x] 4.3 Implement verify_pattern_block(buffer, file_idx, block_idx) returning corrupted byte count
- [ ] 4.4 Unit test pattern generation (write small file, read back, verify)

## 5. UI Module
- [x] 5.1 Create ui.h/ui.c
- [x] 5.2 Implement clear_screen()
- [x] 5.3 Implement draw_header(title)
- [x] 5.4 Implement draw_menu(items, selected_index)
- [x] 5.5 Implement draw_progress(phase, current, total, errors)
- [x] 5.6 Implement draw_results(stats)
- [x] 5.7 Implement draw_prompt(message)

## 6. Menu State
- [x] 6.1 Implement storage selection menu rendering
- [x] 6.2 Implement D-pad navigation
- [x] 6.3 Implement X button to start test
- [x] 6.4 Implement Circle button to exit

## 7. Write Phase
- [x] 7.1 Implement write state initialization (create dir, open first file)
- [x] 7.2 Implement write loop (fill buffer, write, update progress)
- [x] 7.3 Handle file rollover at 1GB boundary
- [x] 7.4 Handle disk full condition
- [x] 7.5 Handle write errors
- [x] 7.6 Support cancellation (Circle button)
- [x] 7.7 Transition to verify phase

## 8. Verify Phase
- [x] 8.1 Implement verify state initialization
- [x] 8.2 Implement verify loop (read buffer, compare pattern, count errors)
- [x] 8.3 Track first bad block location
- [x] 8.4 Handle read errors
- [x] 8.5 Support cancellation
- [x] 8.6 Transition to results phase

## 9. Results & Cleanup
- [x] 9.1 Implement results display (pass/fail, stats)
- [x] 9.2 Implement cleanup prompt
- [x] 9.3 Implement file deletion
- [x] 9.4 Implement exit handling

## 10. Polish & Testing
- [x] 10.1 Add elapsed time tracking
- [ ] 10.2 Test on ux0: (internal storage)
- [ ] 10.3 Test on uma0: (if PSTV available)
- [ ] 10.4 Test cancellation at various points
- [ ] 10.5 Test with small storage to verify disk-full handling
- [x] 10.6 Create README with usage instructions