//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarContactAreaPostprocessor.h"

registerMooseObject("ContactApp", MortarContactAreaPostprocessor);

InputParameters
MortarContactAreaPostprocessor::validParams()
{
  InputParameters params = VolumePostprocessor::validParams();

  params.addClassDescription(
      "Computes the surface area in contact by comparing summing over all lower dimensional blocks "
      "on contact area that have a non-zero Lagrange Multiplier");
  params.addRequiredCoupledVar("variable", "Coupled Lagrange Multiplier");
  params.addParam<Real>(
      "tolerance", TOLERANCE, "The tolerance for accepting that the variable indicates contact");

  return params;
}

MortarContactAreaPostprocessor::MortarContactAreaPostprocessor(const InputParameters & parameters)
  : VolumePostprocessor(parameters),
    _u(adCoupledValue("variable")),
    _tolerance(getParam<Real>("tolerance"))
{
}

Real
MortarContactAreaPostprocessor::computeQpIntegral()
{
  if (_u[_qp] > _tolerance)
    return 1.0;
  else
    return 0.0;
}
