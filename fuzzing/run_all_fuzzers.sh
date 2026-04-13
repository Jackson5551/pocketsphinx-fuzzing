#!/bin/bash
# Run all fuzzers in parallel using AFL++ parallel fuzzing
# This script starts multiple AFL++ instances for better CPU utilization

set -e

# Check if built
if [ ! -f "../build-afl/fuzz_jsgf" ]; then
    echo "Error: Fuzzers not built. Run build-afl.sh first."
    exit 1
fi

# List of fuzzers
FUZZERS=(jsgf ngram audio fsg)

echo "Starting parallel AFL++ fuzzing campaign"
echo "Fuzzers: ${FUZZERS[@]}"
echo ""

# Kill all background jobs on exit
trap 'kill $(jobs -p) 2>/dev/null' EXIT

# Start master fuzzer
MASTER=${FUZZERS[0]}
echo "Starting MASTER fuzzer: $MASTER"
AFL_SKIP_CPUFREQ=1 afl-fuzz -i "seeds/${MASTER}" -o "output/${MASTER}" \
    -M fuzzer01 -- "../build-afl/fuzz_${MASTER}" &

sleep 2

# Start secondary fuzzers
for i in "${!FUZZERS[@]}"; do
    if [ $i -eq 0 ]; then continue; fi  # Skip master

    FUZZER=${FUZZERS[$i]}
    INSTANCE=$(printf "fuzzer%02d" $((i+1)))

    echo "Starting SECONDARY fuzzer: $FUZZER (instance: $INSTANCE)"
    AFL_SKIP_CPUFREQ=1 afl-fuzz -i "seeds/${FUZZER}" -o "output/${FUZZER}" \
        -S "$INSTANCE" -- "../build-afl/fuzz_${FUZZER}" &

    sleep 2
done

echo ""
echo "All fuzzers started in parallel!"
echo "Monitor with: watch -n1 'afl-whatsup output/*/'"
echo "Press Ctrl+C to stop all fuzzers"
echo ""

# Wait for all background jobs
wait
