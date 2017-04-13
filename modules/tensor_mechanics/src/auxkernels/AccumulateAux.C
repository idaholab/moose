/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "AccumulateAux.h"

template <>
InputParameters
validParams<AccumulateAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar(
      "accumulate_from_variable",
      "Variable whose values are to be accumulated into the current variable");
  return params;
}

AccumulateAux::AccumulateAux(const InputParameters & parameters)
  : AuxKernel(parameters), _values(coupledComponents("accumulate_from_variable"))
{
  for (auto i = beginIndex(_values); i < _values.size(); ++i)
    _values[i] = &coupledValue("accumulate_from_variable", i);
}

Real
AccumulateAux::computeValue()
{
  Real ret = _u_old[_qp];
  for (const auto & value : _values)
    ret += (*value)[_qp];
  return ret;
}
