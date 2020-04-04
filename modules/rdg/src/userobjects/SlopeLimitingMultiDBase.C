//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SlopeLimitingMultiDBase.h"

InputParameters
SlopeLimitingMultiDBase::validParams()
{
  InputParameters params = SlopeLimitingBase::validParams();

  params.addClassDescription("Base class for multi-dimensional slope limiting to limit the slopes "
                             "of cell average variables.");

  params.addRequiredParam<UserObjectName>("slope_reconstruction",
                                          "Name of slope reconstruction user object");

  return params;
}

SlopeLimitingMultiDBase::SlopeLimitingMultiDBase(const InputParameters & parameters)
  : SlopeLimitingBase(parameters),
    _rslope(getUserObject<SlopeReconstructionBase>("slope_reconstruction"))
{
}
