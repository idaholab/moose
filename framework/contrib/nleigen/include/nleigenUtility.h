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

#ifndef NLEIGENUTILITY_H
#define NLEIGENUTILITY_H

#include "libmesh/libmesh_config.h"

#if LIBMESH_HAVE_SLEPC

PETSC_EXTERN PetscErrorCode EPSGetOperators_Moose(EPS,Mat*,Mat*);
PETSC_EXTERN PetscErrorCode EPSGetStartVector_Moose(EPS,PetscInt,PetscBool*);
#endif
#endif
