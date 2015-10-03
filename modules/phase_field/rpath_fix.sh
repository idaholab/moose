#!/bin/bash
EXE=$1

# set separator to : temporarily to parse DYLD_LIBRARY_PATH
OIFS=$IFS
IFS=':'

# get libraries that need patching to full path
LIBS=`otool -L $EXE | grep '^\t' | grep -v '^\t/' | awk '{ print $1}' | paste -s -d: -`

for lib in $LIBS
do
  echo fixing $lib
  for path in $DYLD_LIBRARY_PATH
  do
    if [ -e $path/$lib ]
    then
      install_name_tool -change $lib $path/$lib $EXE
      break
    fi
  done
done

# recursively patch all linked libraries
LIBS=`otool -L $EXE | grep '^\t/' | awk '{ print $1}' | paste -s -d: -`

for lib in $LIBS
do
  if [ -w $lib ] && [ $EXE != $lib ]
  then
    echo recursevely fixing $lib
    . rpath_fix.sh $lib
  fi
done

IFS=$OIFS
