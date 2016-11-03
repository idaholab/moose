/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "AccumulateAux.h"

template<>
InputParameters validParams<AccumulateAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("accumulate_from_variable", "Variable whose values are to be accumulated into the current variable");
  return params;
}

AccumulateAux::AccumulateAux(const InputParameters & parameters) :
  AuxKernel(parameters),
  _accumulate_from(coupledValue("accumulate_from_variable"))
{
}

Real
AccumulateAux::computeValue()
{
  if (isNodal())
    return _var.nodalSln()[_qp] + _accumulate_from[_qp];
  else
    return _var.nodalSln()[0] + _accumulate_from[_qp];
}
