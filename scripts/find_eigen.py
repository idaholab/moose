#!/bin/env python
from __future__ import division, print_function , unicode_literals, absolute_import
import warnings
warnings.simplefilter('default', DeprecationWarning)

import os, sys, subprocess

crow_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

eigen_cflags = ""

has_pkg_eigen = subprocess.call(["pkg-config","--exists","eigen3"]) == 0

if has_pkg_eigen:
  eigen_cflags = subprocess.check_output(["pkg-config","eigen3","--cflags"])

libmesh_eigen = os.path.abspath(os.path.join(crow_dir,os.pardir,"moose","libmesh","contrib","eigen","eigen"))

if os.path.exists(libmesh_eigen):
  eigen_cflags = "-I"+libmesh_eigen

if os.path.exists(os.path.join(crow_dir,"contrib","include","Eigen")):
  eigen_cflags = ""

print(eigen_cflags)

