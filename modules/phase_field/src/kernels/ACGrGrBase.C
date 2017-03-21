/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ACGrGrBase.h"

template <>
InputParameters
validParams<ACGrGrBase>()
{
  InputParameters params = ACBulk<Real>::validParams();
  params.addRequiredCoupledVar("v",
                               "Array of coupled order paramter names for other order parameters");
  params.addCoupledVar("T", "temperature");
  return params;
}

ACGrGrBase::ACGrGrBase(const InputParameters & parameters)
  : ACBulk<Real>(parameters),
    _op_num(coupledComponents("v")),
    _vals(_op_num),
    _vals_var(_op_num),
    _mu(getMaterialProperty<Real>("mu")),
    _tgrad_corr_mult(getMaterialProperty<Real>("tgrad_corr_mult")),
    _grad_T(isCoupled("T") ? &coupledGradient("T") : NULL)
{
  // Loop through grains and load coupled variables into the arrays
  for (unsigned int i = 0; i < _op_num; ++i)
  {
    _vals[i] = &coupledValue("v", i);
    _vals_var[i] = coupled("v", i);
  }
}
