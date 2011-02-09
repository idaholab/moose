#!/bin/bash

# Get the current stable version of moose
svn co --quiet https://hpcsc/svn/herd/branches/stable/moose stable-moose

# Merge trunk into the stable version
svn merge https://hpcsc/svn/herd/trunk/moose stable-moose

# Check it in!
svn ci stable-moose

