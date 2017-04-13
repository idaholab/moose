/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
