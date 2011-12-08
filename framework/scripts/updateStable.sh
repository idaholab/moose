#!/bin/bash

hostname=`hostname -s`
ARCH=${ARCH:-intel}

if [ $hostname == 'quark' ]
then
  if [ $ARCH == 'gnu' ]
  then
    # Get the current stable version of moose
    svn co --quiet svn+ssh://hpcsc/herd/trunk/moose stable-moose

    # Merge trunk into the stable version
    svn merge svn+ssh://hpcsc/herd/trunk/devel/moose stable-moose

    # Check it in!
    svn ci --username moosetest stable-moose -m "Auto update at time `date`"
  fi
fi


