# establish cores, or reasonable default
if [ -z "$CPU_COUNT" ]; then
    if [ `uname` == "Darwin" ]; then
        CPU_COUNT=$(echo "$(sysctl -n hw.ncpu) / 2" | bc)
    elif [ `which nproc` ]; then
        CPU_COUNT=$(echo `nproc` / 2 | bc)
    else
        CPU_COUNT=4
    fi
fi
