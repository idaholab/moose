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

#include "LineSearch.h"

using namespace libMesh;

class PetscPingPongLineSearch;
namespace libMesh
{
template <typename>
class PetscNonlinearSolver;
}

template <>
InputParameters validParams<PetscPingPongLineSearch>();

/**
 *  Petsc implementation of the contact line search (based on the Petsc LineSearchShell)
 */
class PetscPingPongLineSearch : public LineSearch
{
public:
  PetscPingPongLineSearch(const InputParameters & parameters);

  virtual void lineSearch() override;
  void timestepSetup() override;

protected:
  PetscNonlinearSolver<Real> * _solver;
  Real _lambda;
  Real _fnorm_older;
  Real _fnorm_old;
  Real _ping_pong_tol;
};

#endif // PETSC_VERSION_LESS_THAN(3, 3, 0)
#endif // LIBMESH_HAVE_PETSC
