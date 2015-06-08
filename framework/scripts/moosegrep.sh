#!/bin/bash
#
# Usage:
#  moosegrep.sh appexecutable objectname inputfile1.i [inputfile2.i ...]
#

APP=$1
OBJ=$2

if [ x$3 == 'x' ]
then
  echo "Usage: moosegrep.sh appexecutable objectname inputfile1.i [inputfile2.i ...]"
  exit 1
fi

n=0
for INPUT in $@
do
  if [ $n -ge 2 ]
  then
    $APP -i $INPUT --list-constructed-objects \
      | sed -n -e '/\*\*START OBJECT DATA\*\*/,/\*\*END OBJECT DATA\*\*/{//!p}' \
      | grep '^'$OBJ'$' > /dev/null 2>&1
    if [ $? -eq 0 ]
    then
      echo $INPUT: $OBJ
    fi
  fi

  let n=n+1
done
