#!/bin/bash

hostname=`hostname -s`

if [ $hostname == 'helios' ]
then
  if [ $ARCH == '-gnu' ]
  then
    # Get the current stable version of moose
    svn co --quiet https://hpcsc/svn/herd/branches/stable/moose stable-moose

    # Merge trunk into the stable version
    svn merge https://hpcsc/svn/herd/trunk/moose stable-moose

    # Check it in!
    svn ci stable-moose -m "Auto update at time `date`"
  fi 
fi


