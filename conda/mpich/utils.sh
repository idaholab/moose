# establish cores, or reasonable default
if [ -z "$CPU_COUNT" ]; then
    if [ `uname` == "Darwin" ]; then
        export CPU_COUNT=$(echo "$(sysctl -n hw.ncpu) / 2" | bc)
    elif [ `which nproc` ]; then
        export CPU_COUNT=$(echo `nproc` / 2 | bc)
    else
        export CPU_COUNT=4
    fi
fi
