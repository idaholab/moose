//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//  This post processor returns the effective saturation of a region.
//
#include "RichardsSeffAux.h"

template <>
InputParameters
validParams<RichardsSeffAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("pressure_vars", "List of variables that represent the pressure");
  params.addRequiredParam<UserObjectName>("seff_UO",
                                          "Name of user object that defines effective saturation.");
  params.addClassDescription("auxillary variable which is effective saturation");
  return params;
}

RichardsSeffAux::RichardsSeffAux(const InputParameters & parameters)
  : AuxKernel(parameters), _seff_UO(getUserObject<RichardsSeff>("seff_UO"))
{
  int n = coupledComponents("pressure_vars");
  _pressure_vals.resize(n);

  for (int i = 0; i < n; ++i)
    _pressure_vals[i] = &coupledValue("pressure_vars", i);
}

Real
RichardsSeffAux::computeValue()
{
  return _seff_UO.seff(_pressure_vals, _qp);
}
