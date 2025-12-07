## ADDED Requirements

### Requirement: Storage Selection Menu
The system SHALL display an interactive menu allowing the user to select a target storage path.

#### Scenario: Available storage devices shown
- **WHEN** the application starts
- **THEN** display a list of mounted storage paths (e.g., `ux0:`, `uma0:`)
- **AND** allow navigation with D-pad up/down
- **AND** confirm selection with X button

#### Scenario: No writable storage
- **WHEN** no writable storage is detected
- **THEN** display an error message
- **AND** allow the user to exit

---

### Requirement: Write Phase
The system SHALL write sequential test files with deterministic patterns until storage is full or user-specified limit is reached.

#### Scenario: Write test files
- **WHEN** write phase begins on target path
- **THEN** create directory `<target>/data/f3vita/`
- **AND** write files named `f3vita_001.dat`, `f3vita_002.dat`, etc.
- **AND** each file SHALL be 1GB (or remaining space if less)
- **AND** each 1MB block within a file SHALL contain pattern: `(file_index << 24) ^ (block_index << 16) ^ byte_offset`

#### Scenario: Storage full
- **WHEN** storage reports no free space
- **THEN** close current file
- **AND** transition to verify phase

#### Scenario: Write error
- **WHEN** a write operation fails
- **THEN** log the error
- **AND** continue to verify phase with files written so far

---

### Requirement: Verify Phase
The system SHALL read back all test files and compare each block against the expected pattern.

#### Scenario: Verify all blocks
- **WHEN** verify phase begins
- **THEN** open each test file in order
- **AND** read each 1MB block
- **AND** compare against expected pattern
- **AND** count mismatched bytes

#### Scenario: Corruption detected
- **WHEN** a block does not match expected pattern
- **THEN** increment corruption counter
- **AND** record first bad block location if not already set
- **AND** continue verification

#### Scenario: Read error
- **WHEN** a read operation fails
- **THEN** treat entire block as corrupted
- **AND** continue with next block

---

### Requirement: Progress Display
The system SHALL show real-time progress during write and verify phases.

#### Scenario: Progress during write
- **WHEN** write phase is active
- **THEN** display: current phase "WRITE"
- **AND** display: MB written / estimated total
- **AND** display: percentage complete
- **AND** update display at least once per second

#### Scenario: Progress during verify
- **WHEN** verify phase is active
- **THEN** display: current phase "VERIFY"
- **AND** display: MB verified / total written
- **AND** display: percentage complete
- **AND** display: errors found so far

---

### Requirement: Final Report
The system SHALL display a summary when verification completes.

#### Scenario: Pass result
- **WHEN** verification completes with zero corrupted bytes
- **THEN** display "PASS" status
- **AND** display total bytes written and verified
- **AND** display elapsed time

#### Scenario: Fail result
- **WHEN** verification completes with corrupted bytes
- **THEN** display "FAIL" status
- **AND** display total bytes corrupted
- **AND** display first bad block location (file and offset)
- **AND** display elapsed time

---

### Requirement: User Cancellation
The system SHALL allow the user to cancel the operation at any time.

#### Scenario: Cancel during write
- **WHEN** user presses Circle button during write phase
- **THEN** stop writing immediately
- **AND** prompt: "Clean up test files? X=Yes, O=No"

#### Scenario: Cancel during verify
- **WHEN** user presses Circle button during verify phase
- **THEN** stop verification
- **AND** display partial results
- **AND** prompt: "Clean up test files? X=Yes, O=No"

---

### Requirement: Cleanup
The system SHALL optionally remove test files after completion.

#### Scenario: Cleanup on request
- **WHEN** user confirms cleanup
- **THEN** delete all `f3vita_*.dat` files in test directory
- **AND** remove `<target>/data/f3vita/` directory if empty
- **AND** display cleanup status

#### Scenario: Skip cleanup
- **WHEN** user declines cleanup
- **THEN** leave test files in place
- **AND** inform user of file locations

---

### Requirement: Exit
The system SHALL exit cleanly when the user is finished.

#### Scenario: Exit from menu
- **WHEN** user presses Circle on storage selection menu
- **THEN** exit application

#### Scenario: Exit after completion
- **WHEN** final report is displayed
- **AND** user presses any button
- **THEN** prompt for cleanup (if not already done)
- **THEN** exit application