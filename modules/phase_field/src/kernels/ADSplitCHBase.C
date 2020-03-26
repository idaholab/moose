//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADSplitCHBase.h"

InputParameters
ADSplitCHBase::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription("Base class for split Cahn-Hilliard equation.");
  return params;
}

ADSplitCHBase::ADSplitCHBase(const InputParameters & parameters) : ADKernel(parameters) {}

ADReal
ADSplitCHBase::computeQpResidual()
{
  return computeDFDC() * _test[_i][_qp];
}

ADReal
ADSplitCHBase::computeDFDC()
{
  return 0.0;
}
