# PocketSphinx AFL++ Fuzzing Harnesses

This directory contains AFL++ fuzzing harnesses for the PocketSphinx speech recognition library. These harnesses target key input parsers that handle untrusted data.

## Attack Vectors

### 1. JSGF Grammar Parser (`fuzz_jsgf`)
**File:** `fuzz_jsgf.c`
**Target:** `src/lm/jsgf.c`, `jsgf_parser.c`, `jsgf_scanner.c`

JSGF (Java Speech Grammar Format) is a complex recursive grammar format. The parser uses Flex/Bison and handles:
- Grammar rule definitions
- Nested alternatives and sequences
- Weight assignments
- Import statements
- Recursive grammar structures

**Potential vulnerabilities:**
- Stack exhaustion from deeply nested rules
- Buffer overflows in string parsing
- Use-after-free in grammar tree construction
- Integer overflows in weight calculations

---

### 2. N-gram Language Model Parser (`fuzz_ngram`)
**File:** `fuzz_ngram.c`
**Target:** `src/lm/ngram_model.c`, `ngram_model_trie.c`

Parses multiple language model formats:
- **ARPA format**: Text-based n-gram models
- **DMP/BIN format**: Binary compressed models
- **Compressed variants**: .gz, .bz2 support

**Potential vulnerabilities:**
- Integer overflows in probability calculations
- Buffer overflows in line parsing
- Memory exhaustion from malicious model sizes
- Format confusion between ARPA/BIN
- Trie structure corruption

---

### 3. Audio File Parser (`fuzz_audio`)
**File:** `fuzz_audio.c`
**Target:** `src/util/soundfiles.c`

Handles audio file format detection and parsing:
- **WAV format**: RIFF/WAVE headers, chunk parsing
- **NIST SPHERE format**: NIST speech data headers
- Format auto-detection via magic bytes

**Potential vulnerabilities:**
- Buffer overflows in header parsing
- Integer overflows in sample count calculations
- Chunk size validation issues
- Unbounded memory allocation based on header fields
- Format confusion attacks

---

### 4. FSG (Finite State Grammar) Parser (`fuzz_fsg`)
**File:** `fuzz_fsg.c`
**Target:** `src/lm/fsg_model.c`

Parses finite state grammar files defining state machines:
- State definitions
- State transitions with probabilities
- Start/final state markers

**Potential vulnerabilities:**
- Invalid state references causing out-of-bounds access
- Cycle detection failures
- Float parsing edge cases
- Graph construction bugs with malformed transitions

---

### 5. Dictionary Parser (`fuzz_dict`)
**File:** `fuzz_dict.c`
**Target:** `src/dict.c`

Parses phonetic dictionaries mapping words to phoneme sequences.

**Potential vulnerabilities:**
- Buffer overflows in line parsing
- Hash table collisions
- Memory exhaustion
- Special character handling issues

---

## Quick Start

### Prerequisites

Install AFL++:
```bash
sudo apt install afl++
```

Or build from source:
```bash
git clone https://github.com/AFLplusplus/AFLplusplus
cd AFLplusplus
make
sudo make install
```

### Build Fuzzers

From the project root:
```bash
./build-afl.sh
```

This will:
1. Configure PocketSphinx with `afl-clang-fast`
2. Enable AddressSanitizer (ASAN) and UndefinedBehaviorSanitizer (UBSAN)
3. Build all fuzzing harnesses
4. Output binaries to `build-afl/`

### Run Individual Fuzzer

```bash
cd fuzzing
./run_fuzzer.sh jsgf
```

Available fuzzers:
- `jsgf` - JSGF grammar parser
- `ngram` - N-gram language model parser
- `audio` - Audio file parser
- `fsg` - FSG parser
- `dict` - Dictionary parser

### Run All Fuzzers in Parallel

```bash
cd fuzzing
./run_all_fuzzers.sh
```

This starts multiple AFL++ instances for better CPU utilization.

Monitor with:
```bash
watch -n1 'afl-whatsup output/*/'
```

### Triage Crashes

After fuzzing finds crashes:
```bash
cd fuzzing
./triage_crashes.sh jsgf
```

This will:
1. Count crashes found
2. Reproduce each crash with ASAN enabled
3. Show hex dump of crash inputs

For detailed debugging:
```bash
gdb --args ../build-afl/fuzz_jsgf
(gdb) r < output/jsgf/default/crashes/id:000000*
```

---

## Fuzzing Strategy

### Persistent Mode

All harnesses use AFL++ persistent mode (`__AFL_LOOP(10000)`) for ~10x performance improvement. Each fuzzer can process 10,000 inputs per fork.

### Sanitizers

Enabled by default:
- **AddressSanitizer (ASAN)**: Detects memory corruption bugs
- **UndefinedBehaviorSanitizer (UBSAN)**: Detects undefined behavior

These significantly increase crash detection but reduce fuzzing speed by ~2-3x.

### Seed Corpus

Minimal valid seeds are provided in `seeds/`:
- `seeds/jsgf/` - Basic and complex JSGF grammars
- `seeds/ngram/` - ARPA format language model
- `seeds/audio/` - Minimal WAV file
- `seeds/fsg/` - Basic FSG file

**Recommendation**: Add your own seeds from:
- PocketSphinx test suite (`test/data/`)
- PocketSphinx model directory (`model/`)
- Real-world grammar/model files

### Dictionary Minimization

Minimize your corpus before long fuzzing runs:
```bash
afl-cmin -i seeds/jsgf -o seeds_min/jsgf -- build-afl/fuzz_jsgf
```

---

## Advanced Configuration

### Timeout

Default timeout is 1000ms. Adjust for slow targets:
```bash
./run_fuzzer.sh jsgf -t 5000+
```

### Memory Limit

Default memory limit is 50MB. Increase if needed:
```bash
./run_fuzzer.sh ngram -m 200
```

### Parallel Fuzzing

Run multiple instances of the same fuzzer:
```bash
# Terminal 1 (master)
afl-fuzz -i seeds/jsgf -o output/jsgf -M fuzzer01 -- build-afl/fuzz_jsgf

# Terminal 2 (secondary)
afl-fuzz -i seeds/jsgf -o output/jsgf -S fuzzer02 -- build-afl/fuzz_jsgf

# Terminal 3 (secondary)
afl-fuzz -i seeds/jsgf -o output/jsgf -S fuzzer03 -- build-afl/fuzz_jsgf
```

### Disable Sanitizers

For faster fuzzing without sanitizers:
```bash
export AFL_USE_ASAN=0
export AFL_USE_UBSAN=0
./build-afl.sh
```

---

## Expected Results

### Coverage

Good fuzzing campaigns should achieve:
- **JSGF**: ~70% edge coverage within 24 hours
- **N-gram**: ~60% edge coverage within 24 hours
- **Audio**: ~50% edge coverage within 12 hours
- **FSG**: ~65% edge coverage within 12 hours

### Bug Classes

Common bug classes found in parsers:
1. **Buffer overflows** - Most common in line/chunk parsing
2. **Integer overflows** - Size calculations, probability math
3. **Use-after-free** - Complex data structure handling
4. **NULL pointer dereferences** - Missing validation
5. **Stack exhaustion** - Recursive parsing
6. **Memory leaks** - Error path handling
7. **Division by zero** - Probability/math calculations

### Reporting Bugs

If you find vulnerabilities:
1. Verify crash is reproducible
2. Minimize test case: `afl-tmin -i crash -o minimized -- build-afl/fuzz_X`
3. Get stack trace with GDB
4. Report to: https://github.com/cmusphinx/pocketsphinx/issues
5. Include: stack trace, minimized input, PocketSphinx version

---

## File Structure

```
fuzzing/
├── README.md                 # This file
├── CMakeLists.txt           # Fuzzer build configuration
├── fuzz_jsgf.c              # JSGF grammar fuzzer
├── fuzz_ngram.c             # N-gram language model fuzzer
├── fuzz_audio.c             # Audio file parser fuzzer
├── fuzz_fsg.c               # FSG parser fuzzer
├── fuzz_dict.c              # Dictionary parser fuzzer
├── run_fuzzer.sh            # Single fuzzer runner
├── run_all_fuzzers.sh       # Parallel fuzzer runner
├── triage_crashes.sh        # Crash reproduction tool
├── seeds/                   # Seed corpus
│   ├── jsgf/
│   ├── ngram/
│   ├── audio/
│   └── fsg/
└── output/                  # AFL++ output (created during fuzzing)
    ├── jsgf/
    ├── ngram/
    ├── audio/
    └── fsg/
```

---

## Performance Tips

1. **Disable CPU frequency scaling**:
   ```bash
   echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
   ```

2. **Increase core dumps**:
   ```bash
   sudo sysctl -w kernel.core_pattern=core
   echo core | sudo tee /proc/sys/kernel/core_pattern
   ```

3. **Use RAM disk for output**:
   ```bash
   mkdir /tmp/ramdisk
   sudo mount -t tmpfs -o size=2G tmpfs /tmp/ramdisk
   # Then use /tmp/ramdisk for output directory
   ```

4. **Monitor with `afl-whatsup`**:
   ```bash
   afl-whatsup -s output/jsgf
   ```

---

## References

- [AFL++ Documentation](https://aflplus.plus/)
- [PocketSphinx](https://github.com/cmusphinx/pocketsphinx)
- [Fuzzing Book](https://www.fuzzingbook.org/)
- [JSGF Specification](https://www.w3.org/TR/jsgf/)

---

## License

Same as PocketSphinx (BSD-like license). See main LICENSE file.
