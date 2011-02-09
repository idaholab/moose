#!/bin/bash

# Get the current stable version of moose
svn co --quiet https://hpcsc/svn/herd/branches/stable/moose stable-moose

# Find the version number
version=`svn info stable-moose | perl -nle 'print $1 if /Last Changed Rev\D*(\d+)/'`

# Merge the trunk into the local copy
svn merge -r $version https://hpcsc/svn/herd/trunk/moose stable-moose

