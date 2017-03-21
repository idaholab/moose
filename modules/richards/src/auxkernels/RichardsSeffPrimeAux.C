/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

//  This post processor returns the effective saturation of a region.
//
#include "RichardsSeffPrimeAux.h"

template <>
InputParameters
validParams<RichardsSeffPrimeAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("pressure_vars", "List of variables that represent the pressure");
  params.addRequiredParam<int>(
      "wrtnum",
      "This aux kernel will return d(seff)/dP_wrtnum.  0<=wrtnum<number_of_pressure_vars.");
  params.addRequiredParam<UserObjectName>("seff_UO",
                                          "Name of user object that defines effective saturation.");
  params.addClassDescription("auxillary variable which is effective saturation");
  return params;
}

RichardsSeffPrimeAux::RichardsSeffPrimeAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _seff_UO(getUserObject<RichardsSeff>("seff_UO")),
    _wrt1(getParam<int>("wrtnum"))
{
  int n = coupledComponents("pressure_vars");
  if (_wrt1 < 0 || _wrt1 >= n)
    mooseError("Your wrtnum is ", _wrt1, " but it must obey 0 <= wrtnum < ", n, ".");
  _pressure_vals.resize(n);

  for (int i = 0; i < n; ++i)
    _pressure_vals[i] = &coupledValue("pressure_vars", i);

  _mat.resize(n);
}

Real
RichardsSeffPrimeAux::computeValue()
{
  _seff_UO.dseff(_pressure_vals, _qp, _mat);
  return _mat[_wrt1];
}
