//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosArrayCoupledArraysForce.h"

registerKokkosResidualObject("MooseTestApp", KokkosArrayCoupledArraysForce);

InputParameters
KokkosArrayCoupledArraysForce::validParams()
{
  InputParameters params = ArrayKernel::validParams();
  params.addRequiredCoupledVar("v", "The coupled array variables which provide the force");
  params.addParam<Real>("coef", 1, "Multiplier for the coupled force term");
  params.addClassDescription("Source term proportional to the sum of several coupled array "
                             "variables, componentwise.");
  return params;
}

KokkosArrayCoupledArraysForce::KokkosArrayCoupledArraysForce(const InputParameters & parameters)
  : ArrayKernel(parameters), _v(kokkosCoupledArrayValues("v")), _coef(getParam<Real>("coef"))
{
  const auto n = coupledComponents("v");

  _v_var.create(n);

  for (const auto k : make_range(n))
  {
    const auto & var = *getArrayVar("v", k);

    if (var.count() != _count)
      paramError("v",
                 "Coupled array variable '",
                 var.name(),
                 "' must have the same number of components as 'variable'");
    if (var.number() == _var.number())
      paramError("v", "Coupled variables need to be different from 'variable'");

    _v_var[k] = var.number();
  }

  _v_var.copyToDevice();
}
