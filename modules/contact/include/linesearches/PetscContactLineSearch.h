//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/libmesh_common.h"
#include "libmesh/petsc_macro.h"

#ifdef LIBMESH_HAVE_PETSC
#if PETSC_VERSION_LESS_THAN(3, 3, 0)
#else

#include "ContactLineSearchBase.h"

using namespace libMesh;

class PetscContactLineSearch;
namespace libMesh
{
template <typename>
class PetscNonlinearSolver;
}

template <>
InputParameters validParams<PetscContactLineSearch>();

/**
 *  Petsc implementation of the contact line search (based on the Petsc LineSearchShell)
 */
class PetscContactLineSearch : public ContactLineSearchBase
{
public:
  PetscContactLineSearch(const InputParameters & parameters);

  virtual void lineSearch() override;

protected:
  PetscNonlinearSolver<Real> * _solver;
};

#endif // PETSC_VERSION_LESS_THAN(3, 3, 0)
#endif // LIBMESH_HAVE_PETSC
