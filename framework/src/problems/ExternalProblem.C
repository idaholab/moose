//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ExternalProblem.h"
#include "NonlinearSystem.h"
#include "AuxiliarySystem.h"

InputParameters
ExternalProblem::validParams()
{
  InputParameters params = FEProblemBase::validParams();
  params.set<bool>("skip_nl_system_check") = true;

  // there is no nonlinear system (we set it as empty in the constructor)
  params.suppressParameter<bool>("ignore_zeros_in_jacobian");
  params.suppressParameter<bool>("kernel_coverage_check");
  params.suppressParameter<std::vector<NonlinearSystemName>>("nl_sys_names");
  params.suppressParameter<bool>("previous_nl_solution_required");
  params.suppressParameter<bool>("skip_nl_system_check");
  params.suppressParameter<bool>("use_nonlinear");

  params.addClassDescription("Problem extension point for wrapping external applications");
  return params;
}

ExternalProblem::ExternalProblem(const InputParameters & parameters) : FEProblemBase(parameters)
{
  /**
   * Ideally the nonlinear system should not exist since we won't ever use it or call solve on it.
   * However, MOOSE currently expects it to exist in several locations throughout the framework.
   * Luckily, it can just be empty (no variables).
   */
  _nl[0] = std::make_shared<NonlinearSystem>(*this, "nl0");
  _aux = std::make_shared<AuxiliarySystem>(*this, "aux0");

  /**
   * We still need to create Assembly objects to hold the data structures for working with Aux
   * Variables, which will be used in the external problem.
   */
  newAssemblyArray(_nl);

  // Create extra vectors and matrices if any
  createTagVectors();

  // Create extra solution vectors if any
  createTagSolutions();
}

void
ExternalProblem::solve(const unsigned int)
{
  TIME_SECTION("solve", 1, "Solving", false)

  syncSolutions(Direction::TO_EXTERNAL_APP);
  externalSolve();
  syncSolutions(Direction::FROM_EXTERNAL_APP);
}
