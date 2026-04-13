#!/bin/bash
# Triage and reproduce crashes found by AFL++

set -e

if [ $# -lt 1 ]; then
    echo "Usage: $0 <fuzzer_name>"
    echo ""
    echo "Available fuzzers: jsgf, ngram, audio, fsg, dict"
    exit 1
fi

FUZZER=$1
OUTPUT_DIR="output/${FUZZER}"
CRASHES_DIR="${OUTPUT_DIR}/default/crashes"

if [ ! -d "$CRASHES_DIR" ]; then
    echo "Error: No crashes directory found: $CRASHES_DIR"
    exit 1
fi

# Count crashes
CRASH_COUNT=$(ls -1 "$CRASHES_DIR" | grep -v README | wc -l)
echo "Found $CRASH_COUNT crash(es) for fuzzer: $FUZZER"
echo ""

if [ $CRASH_COUNT -eq 0 ]; then
    echo "No crashes to triage."
    exit 0
fi

# Reproduce each crash
for crash in "$CRASHES_DIR"/*; do
    if [ -f "$crash" ] && [ "$(basename "$crash")" != "README.txt" ]; then
        echo "============================================"
        echo "Reproducing crash: $(basename "$crash")"
        echo "============================================"

        # Run with ASan to get detailed crash info
        AFL_SKIP_CPUFREQ=1 "../build-afl/fuzz_${FUZZER}" < "$crash" || true

        echo ""
        echo "Crash details:"
        xxd "$crash" | head -20
        echo ""
    fi
done

echo "Triage complete!"
echo ""
echo "To get stack traces, run:"
echo "  gdb --args ../build-afl/fuzz_${FUZZER}"
echo "  (gdb) r < $CRASHES_DIR/<crash_file>"
