//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolutionContainer.h"
#include "NonlinearSystemBase.h"

registerMooseObject("StochasticToolsApp", SolutionContainer);

InputParameters
SolutionContainer::validParams()
{
  InputParameters params = SnapshotContainerBase::validParams();
  params.addClassDescription(
      "Class responsible for collecting distributed solution vectors into a container. We append "
      "a new distributed solution vector (containing all variables) at every execution.");
  MooseEnum system_type("nonlinear aux", "nonlinear");
  params.addParam<MooseEnum>(
      "system", system_type, "The system whose solution should be collected.");

  return params;
}

SolutionContainer::SolutionContainer(const InputParameters & parameters)
  : SnapshotContainerBase(parameters), _system_type(getParam<MooseEnum>("system"))
{
}

std::unique_ptr<NumericVector<Number>>
SolutionContainer::cloneSnapshot()
{
  std::unique_ptr<NumericVector<Number>> cloned_solution;

  // Clone the current solution and append it to the vector
  if (_system_type == "nonlinear")
    cloned_solution =
        _fe_problem.getNonlinearSystemBase(_nonlinear_system_number).solution().clone();
  else
    cloned_solution = _fe_problem.systemBaseAuxiliary().solution().clone();

  return cloned_solution;
}
