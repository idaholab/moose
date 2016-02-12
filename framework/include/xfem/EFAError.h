/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

//This is the only dependency that ElementFragmentAlgorithm has on MOOSE (or libMesh)
//To compile standalone, uncomment the following lines and fix the banned keyword that
//has spaces in it:

//#include <iostream>
//#define EFAError(msg) do {std::c o u t<<"CutElemMesh ERROR: "<<msg<<std::endl; exit(1);} while (0)

//This version just calls MooseError for error reporting, which is preferred if this is run
//within the MOOSE environment:
#include "MooseError.h"
#define EFAError(msg) mooseError(msg)
