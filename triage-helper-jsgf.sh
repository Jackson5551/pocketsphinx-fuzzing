cd ./build-clean

count=0
real_crashes=0
for crash in ../fuzzing/output/jsgf/default/crashes/id:*; do 
    [ -f "$crash" ] || continue
    count=$((count+1))
    timeout 5 ./fuzz_jsgf_triage < "$crash" 2>&1 | grep -qE "ASAN|heap-|stack-|Segmentation|AddressSanitizer" && {
        echo "REAL CRASH: $(basename $crash)"
        ./fuzz_jsgf_triage < "$crash" 2>&1 | head -20
        echo "---"
        real_crashes=$((real_crashes+1))
    }
done
echo ""
echo "Summary: Tested $count crashes, found $real_crashes real bugs"
