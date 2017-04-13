/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "BndsCalcAux.h"

template <>
InputParameters
validParams<BndsCalcAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addClassDescription("Calculate location of grain boundaries in a polycrystalline sample");
  params.addRequiredCoupledVarWithAutoBuild(
      "v", "var_name_base", "op_num", "Array of coupled variables");
  return params;
}

BndsCalcAux::BndsCalcAux(const InputParameters & parameters)
  : AuxKernel(parameters), _op_num(coupledComponents("v")), _vals(_op_num)
{
  for (unsigned int i = 0; i < _op_num; ++i)
    _vals[i] = &coupledValue("v", i);
}

Real
BndsCalcAux::computeValue()
{
  Real value = 0.0;

  for (unsigned int i = 0; i < _op_num; ++i)
    value += (*_vals[i])[_qp] * (*_vals[i])[_qp];

  return value;
}
