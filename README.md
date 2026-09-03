# Two-Pass Assembler in C

A modular two-pass assembler written in ANSI C that translates custom assembly language into binary machine code. The system parses instructions, resolves macro expansions, builds symbol tables, and encodes instructions according to processor architecture specifications.

## Architecture & Pipeline

The compilation process is executed in three main stages:

1. **Pre-Assembler (`Pre_Assembler.c`):**
   * Scans the source `.as` file and expands defined macros.
   * Outputs a clean `.am` file ready for parsing.

2. **First Pass (`First_Pass.c`):**
   * Validates syntax, commands, and operand structures.
   * Allocates memory addresses for data and instruction segments.
   * Builds the comprehensive Symbol Table and tracks external/entry labels.

3. **Second Pass (`Second_Pass.c`):**
   * Resolves label references and memory offsets.
   * Generates binary machine code representations.
   * Produces output files: Object file (`.ob`), Entries file (`.ent`), and Externals file (`.ext`).

## Key Features
* **Strict Memory Management:** Full tracking of dynamic allocations with zero memory leaks.
* **Error Detection:** Identifies semantic and syntactic errors (illegal commas, invalid operands, line-length overflows, undefined labels) with exact line reporting.
* **Dynamic Data Structures:** Custom linked lists and lookup tables for symbols and macro tracking.

## Technologies
* **Language:** ANSI C
* **Build System:** GNU Make
* **Target Environment:** Linux / Unix

## Build & Run

### Compilation
Build the project using the provided `Makefile`:
```bash
make
