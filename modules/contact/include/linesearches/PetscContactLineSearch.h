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

#include "ContactLineSearchBase.h"

using namespace libMesh;

namespace libMesh
{
template <typename>
class PetscNonlinearSolver;
}

/**
 *  Petsc implementation of the contact line search (based on the Petsc LineSearchShell)
 */
class PetscContactLineSearch : public ContactLineSearchBase
{
public:
  static InputParameters validParams();

  PetscContactLineSearch(const InputParameters & parameters);

  virtual void lineSearch() override;

protected:
  PetscNonlinearSolver<Real> * _solver;
};
