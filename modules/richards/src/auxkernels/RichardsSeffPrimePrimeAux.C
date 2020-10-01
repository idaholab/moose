//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//  This post processor returns the 2nd derive of effective saturation.
//
#include "RichardsSeffPrimePrimeAux.h"

registerMooseObject("RichardsApp", RichardsSeffPrimePrimeAux);

InputParameters
RichardsSeffPrimePrimeAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredCoupledVar("pressure_vars", "List of variables that represent the pressure");
  params.addRequiredParam<int>("wrtnum1",
                               "This aux kernel will return d^2(seff)/dP_wrtnum1 "
                               "dP_wrtnum2.  0<=wrtnum1<number_of_pressure_vars.");
  params.addRequiredParam<int>("wrtnum2",
                               "This aux kernel will return d^2(seff)/dP_wrtnum1 "
                               "dP_wrtnum2.  0<=wrtnum2<number_of_pressure_vars.");
  params.addRequiredParam<UserObjectName>("seff_UO",
                                          "Name of user object that defines effective saturation.");
  params.addClassDescription("auxillary variable which is 2nd derivative of effective saturation");
  return params;
}

RichardsSeffPrimePrimeAux::RichardsSeffPrimePrimeAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _seff_UO(getUserObject<RichardsSeff>("seff_UO")),
    _wrt1(getParam<int>("wrtnum1")),
    _wrt2(getParam<int>("wrtnum2")),
    _pressure_vals(coupledValues("pressure_vars"))
{
  int n = coupledComponents("pressure_vars");
  if (_wrt1 < 0 || _wrt1 >= n)
    mooseError("Your wrtnum1 is ", _wrt1, " but it must obey 0 <= wrtnum1 < ", n, ".");
  if (_wrt2 < 0 || _wrt2 >= n)
    mooseError("Your wrtnum2 is ", _wrt2, " but it must obey 0 <= wrtnum2 < ", n, ".");

  _mat.resize(n);
  for (int i = 0; i < n; ++i)
    _mat[i].resize(n);
}

Real
RichardsSeffPrimePrimeAux::computeValue()
{
  _seff_UO.d2seff(_pressure_vals, _qp, _mat);
  return _mat[_wrt1][_wrt2];
}
