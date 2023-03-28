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
      "Class responsible for collecting distributed solution vectors in one place.");
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

void
SolutionContainer::execute()
{
  // Clone the current solution and append it to the vector
  auto cloned_solution = _fe_problem.getNonlinearSystemBase().solution().clone();
  _accumulated_solutions.push_back(std::move(cloned_solution));
}
