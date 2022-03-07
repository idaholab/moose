# echo max cores available, 1 if undeterminable
if [ -z "$CPU_COUNT" ]; then
    if [ `uname` == "Darwin" ]; then
        echo $(echo "$(sysctl -n hw.ncpu) / 2" | bc)
    elif [ `which nproc` ]; then
        echo $(echo `nproc` / 2 | bc)
    else
        echo 1
    fi
fi
