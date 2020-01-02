//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HardCodedPiecewiseLinearFunction.h"

registerMooseObject("MooseTestApp", HardCodedPiecewiseLinearFunction);

InputParameters
HardCodedPiecewiseLinearFunction::validParams()
{
  InputParameters params = PiecewiseLinearBase::validParams();
  params.set<std::vector<Real>>("xy_data") = {}; // Avoids error in base class constructor
  params.addClassDescription(
      "Class to test using setData() to set xy data in a Piecewise function");
  return params;
}

HardCodedPiecewiseLinearFunction::HardCodedPiecewiseLinearFunction(
    const InputParameters & parameters)
  : PiecewiseLinearBase(parameters)
{
  const std::vector<Real> x = {0, 1, 2};
  const std::vector<Real> y = {3, 1, 6};
  setData(x, y);
}
