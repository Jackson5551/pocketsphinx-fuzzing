# PocketSphinx Fuzzing - Quick Start Guide

## TL;DR

```bash
# 1. Install AFL++
sudo apt install afl++

# 2. Build instrumented PocketSphinx
./build-afl.sh

# 3. Run fuzzer
cd fuzzing
./run_fuzzer.sh jsgf

# 4. After finding crashes
./triage_crashes.sh jsgf
```

---

## What Gets Fuzzed?

5 input parsers that handle untrusted data:

1. **JSGF** - Grammar files (`.jsgf`)
2. **N-gram** - Language models (`.arpa`, `.bin`)
3. **Audio** - Sound files (`.wav`, `.nist`)
4. **FSG** - Finite state grammars (`.fsg`)
5. **Dict** - Pronunciation dictionaries

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

---

## What to Expect

### Initial Phase (1-6 hours)
- Fuzzer explores basic code paths
- May find obvious bugs (NULL derefs, simple overflows)
- Corpus grows rapidly

### Middle Phase (6-48 hours)
- Coverage plateaus around 60-70%
- Finds deeper bugs in complex code paths
- Fewer new paths discovered

### Late Phase (48+ hours)
- Minimal new coverage
- May find rare edge cases
- Diminishing returns

### Typical Results
- **Crashes**: 0-50+ depending on code quality
- **Coverage**: 50-70% edge coverage
- **Speed**: 1,000-5,000 exec/sec (with sanitizers)

---

## Interpreting AFL++ UI

```
┌─ Process timing ────────────────────────────────────────────────┐
│        run time : 0 days, 2 hrs, 15 min, 7 sec                 │
│   last new find : 0 days, 0 hrs, 5 min, 32 sec                 │
│ last saved hang : none seen yet                                 │
└─────────────────────────────────────────────────────────────────┘
```
- **run time**: How long fuzzing has been running
- **last new find**: When last interesting input was found
- **last saved hang**: Timeout/hang detection

```
┌─ Overall results ───────────────────────────────────────────────┐
│   cycles done : 12         ← Completed passes through corpus    │
│  corpus count : 847        ← Number of interesting inputs       │
│ saved crashes : 3          ← ⚠️ BUGS FOUND!                     │
│  saved hangs : 0           ← Timeout bugs                       │
└─────────────────────────────────────────────────────────────────┘
```
- **cycles done**: More cycles = more thorough
- **corpus count**: Unique code paths discovered
- **saved crashes**: Found vulnerabilities!

```
┌─ Fuzzing strategy yields ───────────────────────────────────────┐
│   bit flips : 15/64, 12/63, 8/61                                │
│  byte flips : 3/8, 2/7, 1/6                                     │
│ arithmetics : 45/512, 12/463, 3/128                             │
│  known ints : 8/72, 2/248, 0/184                                │
│  dictionary : 0/0, 0/0, 0/0                                     │
│havoc/splice : 234/2.1k, 89/1.8k                                 │
└─────────────────────────────────────────────────────────────────┘
```
- Which mutation strategies are finding new paths
- **havoc/splice**: Usually most effective

```
┌─ Path geometry ─────────────────────────────────────────────────┐
│    levels : 8              ← Depth of path tree                 │
│   pending : 142            ← Inputs waiting to be fuzzed        │
│  pend fav : 12             ← Priority queue                     │
│ own finds : 834            ← Paths found by this instance       │
│  imported : 0              ← Paths from parallel instances      │
│ stability : 100.00%        ← Higher is better (>95% good)       │
└─────────────────────────────────────────────────────────────────┘
```
- **pending**: Should eventually reach 0
- **stability**: Target >95% for reliable fuzzing

---

## Troubleshooting

### "Error: AFL++ not found"
```bash
sudo apt install afl++
# or build from source
```

### "No instrumentation detected"
```bash
# Make sure you ran build-afl.sh, not regular cmake
./build-afl.sh
```

### Fuzzer is slow (<500 exec/sec)
```bash
# Disable sanitizers for speed (but less bug detection)
export AFL_USE_ASAN=0
export AFL_USE_UBSAN=0
./build-afl.sh
```

### "Corpus too small" warning
```bash
# Add more seeds from test suite
cp test/data/*.jsgf fuzzing/seeds/jsgf/
cp test/data/*.arpa fuzzing/seeds/ngram/
```

### No crashes after 24 hours
✅ Good! Code is likely robust. Consider:
- Running longer (48-72 hours)
- Adding more diverse seeds
- Checking coverage (maybe parser isn't reached?)
- Trying without sanitizers (may unlock more paths)

---

## Performance Optimization

### Use RAM disk for output
```bash
mkdir /tmp/ramdisk
sudo mount -t tmpfs -o size=4G tmpfs /tmp/ramdisk
# Edit run_fuzzer.sh to use /tmp/ramdisk/output
```

### Disable CPU frequency scaling
```bash
echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
```

### Run parallel instances
```bash
# Terminal 1
afl-fuzz -i seeds/jsgf -o output/jsgf -M master01 -- build-afl/fuzz_jsgf &

# Terminal 2-4
for i in {2..4}; do
    afl-fuzz -i seeds/jsgf -o output/jsgf -S slave0$i -- build-afl/fuzz_jsgf &
done
```

---

## Attack Vector Priority

If you only have time for one fuzzer:

1. **JSGF** ⭐⭐⭐⭐⭐ - Complex recursive parser, highest bug potential
2. **N-gram** ⭐⭐⭐⭐⭐ - Multiple formats, binary parsing, good target
3. **Audio** ⭐⭐⭐⭐ - RIFF parsing historically buggy
4. **FSG** ⭐⭐⭐ - Simpler parser, medium priority
5. **Dict** ⭐⭐⭐ - Line-based format, lower priority

---

## When You Find Bugs

1. **Verify it's real**:
   ```bash
   ./triage_crashes.sh jsgf
   ```

2. **Minimize test case**:
   ```bash
   afl-tmin -i crash.bin -o crash_min.bin -- build-afl/fuzz_jsgf
   ```

3. **Get stack trace**:
   ```bash
   gdb --args build-afl/fuzz_jsgf
   (gdb) r < crash_min.bin
   (gdb) bt full
   ```

4. **Report**:
   - https://github.com/cmusphinx/pocketsphinx/issues
   - Include: stack trace, minimized input, steps to reproduce

---

## Resources

- **Full Docs**: `fuzzing/README.md`
- **Attack Analysis**: `FUZZING_SUMMARY.md`
- **AFL++ Docs**: https://aflplus.plus/
- **PocketSphinx**: https://github.com/cmusphinx/pocketsphinx

---

## Example Session

```bash
# Start
./build-afl.sh
cd fuzzing
./run_fuzzer.sh jsgf

# [Wait 6-24 hours]

# Check results
ls -la output/jsgf/default/crashes/

# Found crashes!
./triage_crashes.sh jsgf

# Debug
gdb --args ../build-afl/fuzz_jsgf
(gdb) r < output/jsgf/default/crashes/id:000000,sig:06,src:000000,op:havoc,rep:64
(gdb) bt
```

Happy fuzzing! 🐛🔨
