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
  return params;
}

SolutionContainer::SolutionContainer(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _accumulated_solutions(
        declareRestartableData<std::vector<std::unique_ptr<NumericVector<Number>>>>(
            "accumulated_solution"))
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
  auto cloned_solution = _fe_problem.getNonlinearSystemBase().solution().clone();
  _accumulated_solutions.push_back(std::move(cloned_solution));
}
