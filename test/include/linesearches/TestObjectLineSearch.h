//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LineSearch.h"

#include "libmesh/libmesh_common.h"
#include "libmesh/petsc_macro.h"
#include "libmesh/petsc_nonlinear_solver.h"
#include <petscsnes.h>

using namespace libMesh;

/**
 * Minimal LineSearch that accepts the full Newton step. Used to test that each nonlinear
 * system's LineSearch object is dispatched independently by PETSc.
 */
class TestObjectLineSearch : public LineSearch
{
public:
  static InputParameters validParams();

  TestObjectLineSearch(const InputParameters & parameters);

  virtual void lineSearch() override;
};
