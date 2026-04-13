#!/bin/bash
# AFL++ Fuzzing Runner Script
# Usage: ./run_fuzzer.sh <fuzzer_name> [afl_options]
#
# Example: ./run_fuzzer.sh jsgf
# Example: ./run_fuzzer.sh ngram -t 1000+

set -e

if [ $# -lt 1 ]; then
    echo "Usage: $0 <fuzzer_name> [afl_options]"
    echo ""
    echo "Available fuzzers:"
    echo "  jsgf    - JSGF grammar parser"
    echo "  ngram   - N-gram language model parser"
    echo "  audio   - Audio file parser (WAV/NIST)"
    echo "  fsg     - FSG (Finite State Grammar) parser"
    echo "  dict    - Dictionary parser"
    echo ""
    echo "Example: $0 jsgf"
    echo "Example: $0 ngram -t 1000+"
    exit 1
fi

FUZZER=$1
shift
AFL_OPTS="$@"

# Check if built
if [ ! -f "../build-afl/fuzz_${FUZZER}" ]; then
    echo "Error: Fuzzer not built. Run build-afl.sh first."
    exit 1
fi

# Setup directories
SEEDS_DIR="seeds/${FUZZER}"
OUTPUT_DIR="output/${FUZZER}"

if [ ! -d "$SEEDS_DIR" ]; then
    echo "Error: Seeds directory not found: $SEEDS_DIR"
    exit 1
fi

mkdir -p "$OUTPUT_DIR"

# AFL++ settings for better performance
export AFL_SKIP_CPUFREQ=1
export AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1
export AFL_TESTCACHE_SIZE=50

echo "Starting AFL++ fuzzing campaign for: $FUZZER"
echo "Seeds: $SEEDS_DIR"
echo "Output: $OUTPUT_DIR"
echo "Binary: ../build-afl/fuzz_${FUZZER}"
echo ""
echo "Press Ctrl+C to stop fuzzing"
echo ""

# Run AFL++
afl-fuzz -i "$SEEDS_DIR" -o "$OUTPUT_DIR" $AFL_OPTS \
    -- "../build-afl/fuzz_${FUZZER}"
