#!/bin/bash

# Monthly intervals, starting from the open source date, March 10th.
dates=(
2014-03-10
2014-04-10
2014-05-10
2014-06-10
2014-07-10
2014-08-10
2014-09-10
2014-10-10
2014-11-10
2014-12-10
)

# Get abbreviated hash for the first commit older than the given date
hashes=()
for ((i=0; i<${#dates[@]}; i++));
do
  hashes+=(`git log -n1 --pretty="format:%h" --before=${dates[$i]}`)
done

# Report *cumulative* number of unique authors since open source inception.
echo -e "\nCumulative unique author count (since open sourcing)"
for ((i=1; i<${#hashes[@]}; i++));
do
  # The 'tr' command deletes the whitespaces inserted by wc
  n_authors=`git shortlog -s ${hashes[0]}..${hashes[$i]} | wc -l | tr -d ' '`
  echo "${dates[0]}..${dates[$i]}, $n_authors"
done

# We are going to log changes for all files from the top level of the git repository
toplevel=`git rev-parse --show-toplevel`

# Regular expressions defining classes of files to get line change stats for
regexes=(
"*.[Ch]"
"*.py"
)

for ((r=0; r<${#regexes[@]}; r++));
do
    echo -e "\nLines of ${regexes[$r]} files added/deleted:"
    for ((i=1; i<${#hashes[@]}; i++));
    do
        hash1=${hashes[$[$i-1]]}
        hash2=${hashes[$i]}

        # Need to use xargs to limit the length of the argument list to git
        n_lines=`find $toplevel -name "${regexes[$r]}" | xargs git log --numstat --pretty="%H" $hash1..$hash2 | awk 'NF==3 {plus+=$1; minus+=$2} END {printf("+%d, -%d\n", plus, minus)}'`
        echo "${dates[$[$i-1]]}..${dates[$i]}, $n_lines"
    done
done

# Local Variables:
# truncate-lines: t
# End:
