#!/bin/bash

# To test for the number of command line arguments
if [ $# -lt 1 ]; then
  echo "usage: $0 exodus_filename.e"
  echo "Where exodus_filename.e is the file to edit."
  exit 1;
fi

# The file to edit. Note: we don't overwrite it.
infile=$1

# Temporary file to send the results of ncdump to.
dumpfile=tmp.dump

# The output filename is the input filename with "_edited" appended.
outfile="`basename $infile .e`_edited.e"

# You must have ncdump and ncgen in your PATH for this to work!
ncdump $infile > $dumpfile
perl -pli -e's/TRI3/TRISHELL3/g' $dumpfile
ncgen -o $outfile $dumpfile

# Remove temporary file
rm $dumpfile


