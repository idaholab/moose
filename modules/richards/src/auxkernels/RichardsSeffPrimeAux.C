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
#include "RichardsSeffPrimeAux.h"

registerMooseObject("RichardsApp", RichardsSeffPrimeAux);

InputParameters
RichardsSeffPrimeAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
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
    _wrt1(getParam<int>("wrtnum")),
    _pressure_vals(coupledValues("pressure_vars"))
{
  int n = coupledComponents("pressure_vars");
  if (_wrt1 < 0 || _wrt1 >= n)
    mooseError("Your wrtnum is ", _wrt1, " but it must obey 0 <= wrtnum < ", n, ".");

  _mat.resize(n);
}

Real
RichardsSeffPrimeAux::computeValue()
{
  _seff_UO.dseff(_pressure_vals, _qp, _mat);
  return _mat[_wrt1];
}
