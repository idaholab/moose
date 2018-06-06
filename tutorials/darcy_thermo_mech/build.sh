#!/usr/bin/env bash

# Determine the number of jobs
if [ `uname -s` == 'Darwin' ]; then
  JOBS=`/usr/sbin/sysctl -n hw.ncpu`
else
  JOBS=`cat /proc/cpuinfo | grep processor | wc -l`
fi

# Loop through the directories and run make
for dir in */
do
    LOC="${dir%/}"
    echo "make -j ${JOBS} -C ${LOC}"
    make -j ${JOBS} -C ${LOC} "$@"
done
