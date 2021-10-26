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

#include "LineSearch.h"

#include <map>

using namespace libMesh;

class GeometricSearchData;
class PenetrationLocator;
class NonlinearSystemBase;
class DisplacedProblem;

namespace libMesh
{
template <typename>
class PetscNonlinearSolver;
}

/**
 *  Petsc implementation of the contact line search (based on the Petsc LineSearchShell)
 */
class PetscProjectSolutionOntoBounds : public LineSearch
{
public:
  static InputParameters validParams();

  PetscProjectSolutionOntoBounds(const InputParameters & parameters);

  void initialSetup() override;
  virtual void lineSearch() override;

protected:
  NonlinearSystemBase & _nl;
  PetscNonlinearSolver<Real> * _solver;
  DisplacedProblem * _displaced_problem;
  const GeometricSearchData * _geometric_search_data;
  const std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *> *
      _pentration_locators;
};
