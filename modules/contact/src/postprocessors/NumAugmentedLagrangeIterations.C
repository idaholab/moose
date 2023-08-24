//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NumAugmentedLagrangeIterations.h"
#include "AugmentedLagrangianContactProblem.h"

registerMooseObject("ContactApp", NumAugmentedLagrangeIterations);

InputParameters
NumAugmentedLagrangeIterations::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription(
      "Get the number of extra augmented Lagrange loops around the non-linear solve.");
  return params;
}

NumAugmentedLagrangeIterations::NumAugmentedLagrangeIterations(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _augmented_lagrange_problem(
        dynamic_cast<AugmentedLagrangianContactProblemInterface *>(&_fe_problem))

{
}

PostprocessorValue
NumAugmentedLagrangeIterations::getValue() const
{
  return _augmented_lagrange_problem ? _augmented_lagrange_problem->getLagrangianIterationNumber()
                                     : 0;
}
