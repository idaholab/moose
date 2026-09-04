//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RemeshCount.h"

#include "FEProblemBase.h"
#include "Remeshing.h"

registerMooseObject("MooseApp", RemeshCount);

InputParameters
RemeshCount::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Return the cumulative number of remesh events performed so far.");
  return params;
}

RemeshCount::RemeshCount(const InputParameters & parameters) : GeneralPostprocessor(parameters) {}

Real
RemeshCount::getValue() const
{
  // Reporting zero rather than erroring keeps this postprocessor usable in an input whose
  // [Remeshing] block is absent or commented out
  if (!_fe_problem.hasRemeshing())
    return 0.0;

  return _fe_problem.getRemeshing().remeshCount();
}
