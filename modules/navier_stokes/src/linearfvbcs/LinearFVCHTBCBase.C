//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVCHTBCBase.h"

InputParameters
LinearFVCHTBCBase::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionBC::validParams();
  params.addRangeCheckedParam<Real>(
      "temperature_relaxation",
      1.0,
      "0.0<temperature_relaxation & temperature_relaxation<=1.0",
      "The relaxation parameter for the temperature field in the coupling.");

  params.addRangeCheckedParam<Real>("flux_relaxation",
                                    1.0,
                                    "0.0<flux_relaxation & flux_relaxation<=1.0",
                                    "The relaxation parameter for the flux field in the coupling.");

  return params;
}

LinearFVCHTBCBase::LinearFVCHTBCBase(const InputParameters & parameters)
  : LinearFVAdvectionDiffusionBC(parameters),
    _temperature_relaxation_factor(getParam<Real>("temperature_relaxation")),
    _flux_relaxation_factor(getParam<Real>("flux_relaxation"))
{
}
