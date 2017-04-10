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

#ifndef MONOLITH_H
#define MONOLITH_H

#include "libmesh/libmesh_config.h"

#if LIBMESH_HAVE_SLEPC

#include <slepc/private/epsimpl.h> /*I "slepceps.h" I*/

PETSC_EXTERN PetscErrorCode EPSCreate_Monolith(EPS);

#endif

#endif
