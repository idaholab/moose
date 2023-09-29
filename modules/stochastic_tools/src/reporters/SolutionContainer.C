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
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription(
      "Class responsible for collecting distributed solution vectors into a container. We append "
      "a new distributed solution vector (containing all variables) at every execution.");
  MooseEnum system_type("nonlinear aux", "nonlinear");
  params.addParam<MooseEnum>(
      "system", system_type, "The system whose solution should be collected.");
  params.addParam<NonlinearSystemName>(
      "nonlinear_system_name",
      "nl0",
      "Option to select which nonlinear system's solution shall be stored.");
  return params;
}

SolutionContainer::SolutionContainer(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _accumulated_solutions(
        declareRestartableData<std::vector<std::unique_ptr<NumericVector<Number>>>>(
            "accumulated_solution")),
    _system_type(getParam<MooseEnum>("system")),
    _nonlinear_system_number(
        _fe_problem.nlSysNum(getParam<NonlinearSystemName>("nonlinear_system_name")))
{
}

void
SolutionContainer::initialSetup()
{
  _accumulated_solutions.clear();
}

const std::unique_ptr<NumericVector<Number>> &
SolutionContainer::getSolution(unsigned int local_i) const
{
  mooseAssert(local_i < _accumulated_solutions.size(),
              "The container only has (" + std::to_string(_accumulated_solutions.size()) +
                  ") solutions so we cannot find any with index (" + std::to_string(local_i) +
                  ")!");
  return _accumulated_solutions[local_i];
}

void
SolutionContainer::execute()
{
  // Clone the current solution and append it to the vector
  if (_system_type == "nonlinear")
  {
    auto cloned_solution =
        _fe_problem.getNonlinearSystemBase(_nonlinear_system_number).solution().clone();
    _accumulated_solutions.push_back(std::move(cloned_solution));
  }
  else
  {
    auto cloned_solution = _fe_problem.systemBaseAuxiliary().solution().clone();
    _accumulated_solutions.push_back(std::move(cloned_solution));
  }
}
