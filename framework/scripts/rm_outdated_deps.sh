#!/bin/bash

# You can run the script from your animal or from the "trunk"
# directory.  Though it may take quite a long time to run from trunk.
dep_file_list=`find . -name "*.d"`

for depfile in $dep_file_list; do
  # Reset the done flag
  depfile_done=0

  # Read dependency file, look for files which no longer exist
  while read line
  do 
    # Look at each "word" in the line
    for possible_header in $line
    do
      # Is it a header file? This uses bash regular expressions.
      if [[ $possible_header =~ \.h$ ]]; then

        # Does the file exist
        if [ ! -f $possible_header ]; then
          # We can skip the rest of this file, which means breaking
          # out of two loops... unfortunately there's no goto statement in bash.
          depfile_done=1
          break # out of for loop
        fi
      fi
    done # for possible_header

    # See if we can skip the rest of this dependency file and just delete it for being outdated
    if [ $depfile_done -eq 1 ]; then
      echo "$depfile is out of date, removing it"
      rm -f $depfile
      
      # Removing only the dependency file may be insufficient, and may in fact lead to
      # an incorrect build -- we also need to remove the object file associated with the
      # dependency file we removed...we can hopefully get the name of the object file by
      # stripping off the .d from the dependency file's filename.
      objfile=`echo $depfile | rev | cut -c 3- | rev`
      rm -f $objfile

      break # out of while loop
    fi

  done < $depfile # while read line

done # for each depfile
