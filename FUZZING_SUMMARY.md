# PocketSphinx AFL++ Fuzzing Campaign - Summary

## Overview

This fuzzing infrastructure targets **PocketSphinx**, a speech recognition library, with focus on input parsers that handle untrusted data. Five dedicated harnesses have been created to maximize code coverage and bug discovery.

---

## Attack Surface Analysis

### Priority 1: High-Value Targets

#### 1. **JSGF Grammar Parser** ⭐⭐⭐⭐⭐
- **Complexity**: Very High
- **Risk**: High (recursive parser, complex state machine)
- **Files**: `src/lm/jsgf.c`, `jsgf_parser.c` (Bison), `jsgf_scanner.c` (Flex)
- **Why Target**:
  - Flex/Bison parsers are historically bug-prone
  - Recursive grammar handling prone to stack exhaustion
  - Complex string manipulation with weight parsing
  - Used when loading user-provided grammar files

#### 2. **N-gram Language Model Parser** ⭐⭐⭐⭐⭐
- **Complexity**: Very High
- **Risk**: High (multiple formats, binary parsing, compression)
- **Files**: `src/lm/ngram_model.c`, `ngram_model_trie.c`, `lm_trie_quant.c`
- **Why Target**:
  - Handles both text (ARPA) and binary (DMP/BIN) formats
  - Complex trie data structures with quantization
  - Size calculations vulnerable to integer overflows
  - Probability calculations with floating point edge cases
  - Used when loading language models from files

#### 3. **Audio File Parser** ⭐⭐⭐⭐
- **Complexity**: Medium-High
- **Risk**: Medium-High (header parsing, chunk handling)
- **Files**: `src/util/soundfiles.c`
- **Why Target**:
  - WAV RIFF chunk parsing notoriously buggy
  - NIST SPHERE format less tested
  - Header size fields control memory allocation
  - Format auto-detection can be confused
  - Direct exposure when processing user audio files

---

### Priority 2: Medium-Value Targets

#### 4. **FSG Parser** ⭐⭐⭐
- **Complexity**: Medium
- **Risk**: Medium (state machine construction)
- **Files**: `src/lm/fsg_model.c`
- **Why Target**:
  - State machine graph construction
  - State reference validation issues
  - Probability/weight parsing
  - Used for grammar-based recognition

#### 5. **Dictionary Parser** ⭐⭐⭐
- **Complexity**: Medium
- **Risk**: Medium (line parsing, hash tables)
- **Files**: `src/dict.c`
- **Why Target**:
  - Simple line-based format
  - Hash table construction with collision handling
  - Phoneme string parsing
  - Used when loading pronunciation dictionaries

---

## Vulnerability Classes Expected

Based on code patterns observed:

### 1. Memory Safety Issues
- **Buffer overflows** in string parsing functions
  - `strcpy`, `sprintf` usage without bounds checking
  - Fixed-size buffers for line reading

- **Heap overflows** in dynamic allocations
  - Size calculations from untrusted input
  - Missing overflow checks before `malloc/calloc`

- **Use-after-free** in complex data structures
  - Grammar trees, trie structures
  - Error path cleanup

### 2. Integer Vulnerabilities
- **Integer overflows** in:
  - Model size calculations (n-gram counts × size)
  - Audio sample counts (channels × samples × bytes)
  - Probability calculations

- **Signedness issues**:
  - Size fields read as signed but used as unsigned
  - Negative indices in array access

### 3. Logic Bugs
- **Format confusion**: ARPA vs BIN model detection
- **State machine bugs**: Invalid state transitions in FSG/JSGF
- **Resource exhaustion**: Unbounded allocations

### 4. Parser-Specific Issues
- **Stack exhaustion**: Deep recursion in JSGF parser
- **Null pointer dereferences**: Missing validation after `malloc`
- **Division by zero**: Probability normalization

---

## Implementation Details

### Fuzzer Architecture

All harnesses follow this pattern:

```c
__AFL_FUZZ_INIT();

int main() {
    __AFL_INIT();  // AFL++ persistent mode
    unsigned char *buf = __AFL_FUZZ_TESTCASE_BUF;

    while (__AFL_LOOP(10000)) {  // Process 10k inputs per fork
        int len = __AFL_FUZZ_TESTCASE_LEN;

        // Write to temp file (PocketSphinx uses file-based API)
        char tmpfile[] = "/tmp/fuzz_XXXXXX";
        int fd = mkstemp(tmpfile);
        write(fd, buf, len);
        close(fd);

        // Call target parser
        parse_function(tmpfile);

        // Cleanup
        unlink(tmpfile);
    }
}
```

### Sanitizer Configuration

**AddressSanitizer (ASAN)** enabled:
- Heap buffer overflow detection
- Stack buffer overflow detection
- Use-after-free detection
- Double-free detection
- Memory leak detection

**UndefinedBehaviorSanitizer (UBSAN)** enabled:
- Integer overflow detection
- Division by zero
- NULL pointer dereferences
- Alignment violations

### Build Configuration

```cmake
CC=afl-clang-fast
CXX=afl-clang-fast++
CMAKE_BUILD_TYPE=Debug
BUILD_SHARED_LIBS=OFF
AFL_USE_ASAN=1
AFL_USE_UBSAN=1
```

---

## Seed Corpus Strategy

### JSGF Seeds
- **basic.jsgf**: Simple grammar with alternatives
- **complex.jsgf**: Multi-rule grammar with recursion
- **Additional**: Extract from `test/data/` directory

### N-gram Seeds
- **basic.arpa**: Minimal 2-gram ARPA format
- **Additional**: Use model files from `model/` directory
- **Binary**: Convert ARPA to BIN using `pocketsphinx_lm_convert`

### Audio Seeds
- **minimal.wav**: Valid WAV with minimal header
- **Additional**: Extract from test suite audio files
- **Variations**: Different sample rates, channels, bit depths

### FSG Seeds
- **basic.fsg**: Simple 3-state FSG
- **Additional**: Convert JSGF to FSG using built tools

---

## Fuzzing Campaign Strategy

### Phase 1: Initial Discovery (24-48 hours)
Run all fuzzers in parallel with sanitizers enabled:
```bash
./build-afl.sh
cd fuzzing
./run_all_fuzzers.sh
```

**Expected**: Quick crashes from obvious bugs (NULL derefs, simple overflows)

### Phase 2: Deep Coverage (1-2 weeks)
Focus on high-priority targets (JSGF, N-gram):
```bash
# Multiple instances per target
for i in {1..4}; do
    afl-fuzz -i seeds/jsgf -o output/jsgf -S fuzzer0$i -- build-afl/fuzz_jsgf &
done
```

**Expected**: Edge cases, integer overflows, complex state machine bugs

### Phase 3: Corpus Minimization & Reproduction
```bash
# Minimize corpus
afl-cmin -i output/jsgf/*/queue -o corpus_min/jsgf -- build-afl/fuzz_jsgf

# Minimize individual crashes
afl-tmin -i crash.bin -o crash_min.bin -- build-afl/fuzz_jsgf
```

### Phase 4: Verification & Reporting
- Reproduce crashes in clean environment
- Generate stack traces with GDB
- Create minimal proof-of-concepts
- Report to upstream

---

## Performance Expectations

### Coverage Targets (24 hours)
- **JSGF**: 60-70% edge coverage
- **N-gram**: 55-65% edge coverage
- **Audio**: 45-55% edge coverage
- **FSG**: 60-70% edge coverage

### Throughput
With sanitizers:
- **JSGF**: ~1,000-2,000 exec/sec
- **N-gram**: ~800-1,500 exec/sec
- **Audio**: ~3,000-5,000 exec/sec (simpler parser)

Without sanitizers (2-3x faster):
- **JSGF**: ~3,000-5,000 exec/sec
- **N-gram**: ~2,000-3,000 exec/sec

### Resource Requirements
- **CPU**: 4-8 cores minimum for parallel fuzzing
- **RAM**: 2-4 GB per fuzzer instance
- **Disk**: 10-20 GB for corpus + crashes (use tmpfs)

---

## Integration Points

### Continuous Fuzzing
Can integrate with:
- **OSS-Fuzz**: Submit to Google's OSS-Fuzz for continuous fuzzing
- **ClusterFuzz**: Internal continuous fuzzing infrastructure
- **CI/CD**: Run 1-hour fuzzing campaigns on each PR

### Regression Testing
Save interesting corpus:
```bash
# After fuzzing campaign
afl-cmin -i output/*/queue -o regression_corpus -- build-afl/fuzz_X

# Add to CI
for seed in regression_corpus/*; do
    ./build-afl/fuzz_X < $seed || exit 1
done
```

---

## Known Limitations

1. **File-based API**: PocketSphinx primarily uses file-based APIs, requiring tmpfile creation (slower than in-memory)

2. **Dictionary fuzzer**: Requires valid acoustic model to fully test, currently limited

3. **Binary formats**: N-gram BIN format harder to fuzz effectively (consider grammar-based fuzzing)

4. **Compression**: .gz/.bz2 compressed models harder to mutate effectively

---

## Next Steps

1. **Run initial 48-hour campaign** to identify low-hanging fruit
2. **Expand corpus** with real-world grammars/models from test suite
3. **Consider grammar-based fuzzing** for structured formats (ARPA, FSG)
4. **Add coverage-guided harnesses** for specific complex functions
5. **Submit to OSS-Fuzz** for continuous fuzzing

---

## Quick Reference

```bash
# Build
./build-afl.sh

# Run single fuzzer
cd fuzzing && ./run_fuzzer.sh jsgf

# Run all fuzzers
cd fuzzing && ./run_all_fuzzers.sh

# Monitor progress
afl-whatsup output/*/

# Triage crashes
cd fuzzing && ./triage_crashes.sh jsgf

# Debug crash
gdb --args ../build-afl/fuzz_jsgf
(gdb) r < output/jsgf/default/crashes/id:000000*
```

---

## Contact

For questions about this fuzzing infrastructure:
- Create issue on GitHub
- Check AFL++ docs: https://aflplus.plus/

For PocketSphinx bugs:
- Report to: https://github.com/cmusphinx/pocketsphinx/issues
