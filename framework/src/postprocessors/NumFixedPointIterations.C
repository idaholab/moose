//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "NumFixedPointIterations.h"
#include "MooseApp.h"
#include "Executioner.h"

registerMooseObject("MooseApp", NumFixedPointIterations);
registerMooseObjectRenamed("MooseApp",
                           NumPicardIterations,
                           "06/30/2021 24:00",
                           NumFixedPointIterations);

InputParameters
NumFixedPointIterations::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription(
      "Returns the number of fixed point iterations taken by the executioner.");
  params.addParam<bool>("get_index_instead_of_count",
                        false,
                        "If true, get the current fixed point iteration index instead of the "
                        "number of fixed point iterations taken, shifting by -1");
  return params;
}

NumFixedPointIterations::NumFixedPointIterations(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _count_shift(getParam<bool>("get_index_instead_of_count") ? -1 : 0)
{
}

Real
NumFixedPointIterations::getValue() const
{
  return _app.getExecutioner()->fixedPointSolve().numFixedPointIts() + _count_shift;
}
