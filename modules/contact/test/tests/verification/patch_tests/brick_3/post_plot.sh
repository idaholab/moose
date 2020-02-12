#!/bin/bash

NO_ARGS=1

# Flag to keep generated files
keep="no"

usage ()
{
    echo "Usage `basename $0` options (-hk)"
    echo "       -h This help"
    echo "       -k keep generated files"
    exit
}

# Process commandline arguments, if any
if [ $# -ge "$NO_ARGS" ]
then
  while getopts ":hk" Option
  do
    case $Option in
	h ) usage;;
	k ) keep="yes";;
    esac
  done
fi

# Function to cleanup generated files
cleanup ()
{
    rm -f *.dat *.xmgr *.met *.plt *.log gold/*.dat gold/*.xmgr gold/*.met
}

# Gnuplot formatting for specific plots to be generated
gnuplot_header="set terminal postscript enhanced color"
title[0]="Displacement Plot"
title[1]="Iteration Plot"
title[2]="Reaction Plot"
title[3]="Contact Pressure Plot (Time = 1.0)"

ylabel[0]="Displacement"
ylabel[1]="Iterations"
ylabel[2]="Reaction"
ylabel[3]="Pressure"

xlabel="Time"
xlabel2="Distance"

# Gnuplot line formatting, curr output and gold output
cfmt="w lp lt 2 pt 2 lc 3"
gfmt="w lp lt 1 pt 4 lc 2"

# Select CSV files generated for plotting
csv_files="*out.csv"

# Main loop for processing files
for file in $csv_files
do
    echo "Generating plot files for ${file}"
    base=${file/%\.csv/}
    datfile="${base}.dat"
    datfile2="${base}.xmgr"
    pltfile="${base}.plt"
    logfile="${base}.log"
    psfile="${base}.ps"

# Create data files for Gnuplot from CSV files
    perl -ple 's/^time/# time/' ${file} > ${datfile}
    perl -i -ple 's/,/ /g' ${datfile}
    if [ -d "gold" ]
    then
      perl -ple 's/^time/# time/' gold/${file} > gold/${datfile}
      perl -i -ple 's/,/ /g' gold/${datfile}
    fi

# Generate interface pressure data using BLOT
    blot --device cps --input blot_splot.inp ${base}.e > ${logfile} 2>&1
    cd gold
    blot --device cps --input ../blot_splot.inp ${base}.e >> ../${logfile} 2>&1
    perl -i -ple 's/^@/#@/g' ${datfile2}
    perl -i -ple 's/^&/#&/g' ${datfile2}
    cd ..
    perl -i -ple 's/^@/#@/g' ${datfile2}
    perl -i -ple 's/^&/#&/g' ${datfile2}
 
# Check for already existing plot files
    if [ -e ${pltfile} ]
    then
	echo "${pltfile} already exists!"
	echo "Remove *.plt files by hand."
	exit
    fi

# Create plot files for Gnuplot
    echo "$gnuplot_header" > ${pltfile}
    echo "set title \"${title[0]}\"" >> ${pltfile}
    echo "set output \"${psfile}\"" >> ${pltfile}
    echo "set xlabel \"$xlabel\"" >> ${pltfile}
    echo "set ylabel \"${ylabel[0]}\"" >> ${pltfile}
    echo "plot \"${datfile}\" using 1:5 t \"Node 28 (x-disp,curr)\" $cfmt,\\" >> ${pltfile}
    echo "\"gold/${datfile}\" using 1:5 t \"Node 28 (x-disp,gold)\" $gfmt" >> ${pltfile}
    echo "plot \"${datfile}\" using 1:6 t \"Node 33 (x-disp,curr)\" $cfmt,\\" >> ${pltfile}
    echo "\"gold/${datfile}\" using 1:6 t \"Node 33 (x-disp,gold)\" $gfmt" >> ${pltfile}
    echo "plot \"${datfile}\" using 1:7 t \"Node 28 (y-disp,curr)\" $cfmt,\\" >> ${pltfile}
    echo "\"gold/${datfile}\" using 1:7 t \"Node 28 (y-disp,gold)\" $gfmt" >> ${pltfile}
    echo "plot \"${datfile}\" using 1:8 t \"Node 33 (y-disp,curr)\" $cfmt,\\" >> ${pltfile}
    echo "\"gold/${datfile}\" using 1:8 t \"Node 33 (y-disp,gold)\" $gfmt" >> ${pltfile}
    echo "set title \"${title[1]}\"" >> ${pltfile}
    echo "set xlabel \"$xlabel\"" >> ${pltfile}
    echo "set ylabel \"${ylabel[1]}\"" >> ${pltfile}
    echo "plot \"${datfile}\" using 1:9 t \"Lin It (curr)\" $cfmt,\\" >> ${pltfile}
    echo "\"${datfile}\" using 1:10 t \"NonLin It (curr)\" $cfmt,\\" >> ${pltfile}
    echo "\"gold/${datfile}\" using 1:9 t \"Lin It (gold)\" $gfmt,\\" >> ${pltfile}
    echo "\"gold/${datfile}\" using 1:10 t \"NonLin It (gold)\" $gfmt" >> ${pltfile}
    echo "set title \"${title[2]}\"" >> ${pltfile}
    echo "set xlabel \"$xlabel\"" >> ${pltfile}
    echo "set ylabel \"${ylabel[2]}\"" >> ${pltfile}
    echo "plot \"${datfile}\" using 1:3 t \"Bot React X (curr)\" $cfmt,\\" >> ${pltfile}
    echo "\"gold/${datfile}\" using 1:3 t \"Bot React X (gold)\" $gfmt" >> ${pltfile}
    echo "plot \"${datfile}\" using 1:4 t \"Bot React Y (curr)\" $cfmt,\\" >> ${pltfile}
    echo "\"gold/${datfile}\" using 1:4 t \"Bot React Y (gold)\" $gfmt" >> ${pltfile}
    echo "plot \"${datfile}\" using 1:15 t \"Top React X (curr)\" $cfmt,\\" >> ${pltfile}
    echo "\"gold/${datfile}\" using 1:15 t \"Top React X (gold)\" $gfmt" >> ${pltfile}
    echo "plot \"${datfile}\" using 1:16 t \"Top React Y (curr)\" $cfmt,\\" >> ${pltfile}
    echo "\"gold/${datfile}\" using 1:16 t \"Top React Y (gold)\" $gfmt" >> ${pltfile}
    echo "set title \"${title[3]}\"" >> ${pltfile}
    echo "set xlabel \"$xlabel2\"" >> ${pltfile}
    echo "set ylabel \"${ylabel[3]}\"" >> ${pltfile}
    echo "plot \"${datfile2}\" using 1:2 t \"Contact Pressure (curr)\" $cfmt,\\" >> ${pltfile}
    echo "\"gold/${datfile2}\" using 1:2 t \"Contact Pressure (gold)\" $gfmt" >> ${pltfile}

# Execute Gnuplot on generated file
    gnuplot < ${pltfile} >> ${logfile} 2>&1
done

# Cleanup files
if [ "$keep" != "yes" ]
then
    cleanup
fi

exit 0
