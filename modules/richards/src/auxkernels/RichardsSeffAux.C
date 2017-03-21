/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
