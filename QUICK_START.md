# PocketSphinx Fuzzing - Quick Start Guide
File created by AI to aid in helping anyone get started with fuzzing this project.
## TL;DR

```bash
# 1. Install AFL++
# Go to the AFL++ docs for more info
---

## What Gets Fuzzed?

5 input parsers that handle untrusted data:

1. **JSGF** - Grammar files (`.jsgf`) - Fuzeed - Report Submitted
2. **N-gram** - Language models (`.arpa`, `.bin`) - Not Yet Fuzzed
3. **Audio** - Sound files (`.wav`, `.nist`) - Fuzzed - Report not Submitted
4. **FSG** - Finite state grammars (`.fsg`) - Not Yet Fuzzed
5. **Dict** - Pronunciation dictionaries - Not Yet Fuzzed

---

## File Layout

```
pocketsphinx/
├── build-afl.sh              # Build script
├── FUZZING_SUMMARY.md        # Detailed attack surface analysis
└── fuzzing/
    ├── README.md             # Full documentation
    ├── QUICK_START.md        # This file
    ├── fuzz_*.c              # 5 fuzzing harnesses
    ├── run_fuzzer.sh         # Run single fuzzer
    ├── run_all_fuzzers.sh    # Run all fuzzers
    ├── triage_crashes.sh     # Analyze crashes
    └── seeds/                # Initial test cases
```

---

## Common Commands

### Build
```bash
./build-afl.sh
```
### Clean Build
```
cd build_clean

cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_C_FLAGS="-fsanitize=address -fno-omit-frame-pointer -g" \
      ..
      
make pocketsphinx -j4

# Build triage fuzzer
afl-cc -fsanitize=address -fno-omit-frame-pointer -g \
    -I../include -Iinclude \
    ../fuzzing/fuzz_jsgf.c \
    -o fuzz_jsgf_triage \
    libpocketsphinx.a -lm
```

### Run Individual Fuzzer
```bash
cd fuzzing
./run_fuzzer.sh jsgf          # JSGF grammar parser
./run_fuzzer.sh ngram         # Language model parser
./run_fuzzer.sh audio         # Audio file parser
./run_fuzzer.sh fsg           # FSG parser
```

### Run All Fuzzers in Parallel
```bash
cd fuzzing
./run_all_fuzzers.sh
```

### Monitor Progress
```bash
watch -n1 'afl-whatsup output/*/'
```

### Check Crashes
```bash
cd fuzzing
./triage_crashes.sh jsgf
```

### Debug Crash
```bash
gdb --args build-afl/fuzz_jsgf
(gdb) r < fuzzing/output/jsgf/default/crashes/id:000000*
(gdb) bt
```

## Attack Vector Priority

These are our planned attack vectors:

1. **JSGF** - Complex recursive parser, highest bug potential
2. **N-gram** - Multiple formats, binary parsing, good target
3. **Audio** - RIFF parsing historically buggy
4. **FSG** - Simpler parser, medium priority
5. **Dict** - Line-based format, lower priority

---

## When You Find Bugs

1. **Verify it's real**:
   ```bash
   ./triage_crashes.sh jsgf
   ```

2. **Minimize test case (if you want)**:
   ```bash
   afl-tmin -i crash.bin -o crash_min.bin -- build-afl/fuzz_jsgf
   ```

3. **Get stack trace**:
   I have created a simple `triage-helper-jsgf.sh` file that can be adapted to be used for other fuzzers.
   Run that after triaging crashes and it will print the stack trace with some additional information.
---

## Resources

- **AFL++ Docs**: https://aflplus.plus/
- **PocketSphinx**: https://github.com/cmusphinx/pocketsphinx