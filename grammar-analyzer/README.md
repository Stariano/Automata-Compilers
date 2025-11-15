# Grammar Analyzer (Compiler Project 2)

This folder holds the grammar analysis utilities I built for the second compiler assignment (CSE340 Project 2). It reads a textbook-style grammar, lists terminals/nonterminals, and validates parsing tasks.

## Contents
- `project2.cc` is the driver: it advances tokens, records symbol order, and implements the parsing logic described in `CSE340F25_Proj2.pdf`.
- `lexer.cc`, `lexer.h`, `inputbuf.cc`, and `inputbuf.h` keep tokenization in one place so the parser logic never touches raw bytes.
- `tests/` contains 12 numbered input files along with multiple `.expected` files per input so you can reproduce the autograder checks; `test_p2.sh` automates running them against the current executable.
- The binary `project2` (and `a.out`) are already built for quick verification.

## Running the analysis
```bash
cd theory/grammar-analyzer
g++ -std=c++17 project2.cc lexer.cc inputbuf.cc -o grammar-analyzer
./test_p2.sh ./grammar-analyzer
```
Check any `.expected*` file to compare what the homework expected for each terminal/nonterminal listing.
