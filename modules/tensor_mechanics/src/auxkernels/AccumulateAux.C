//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
