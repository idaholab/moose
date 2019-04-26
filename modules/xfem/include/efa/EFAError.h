//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
// This is the only dependency that ElementFragmentAlgorithm has on MOOSE (or libMesh)
// To compile standalone, uncomment the following lines and fix the banned keyword that
// has spaces in it:

//#include <iostream>
//#define EFAError(msg) do {std::c o u t, "CutElemMesh ERROR: ", msg, std::endl; exit(1);} while (0)
//#define EFAWarning(msg) do {std::c o u t<<"CutElemMesh WARNING: "<<msg<<std::endl;} while (0)

// This version just calls MooseError for error reporting, which is preferred if this is run
// within the MOOSE environment:
#include "MooseError.h"
#define EFAError(...) mooseError(__VA_ARGS__)
#define EFAWarning(...) mooseWarning(__VA_ARGS__)
