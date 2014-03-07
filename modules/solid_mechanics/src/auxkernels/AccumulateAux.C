/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "AccumulateAux.h"

template<>
InputParameters validParams<AccumulateAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("accumulate_from_variable","Variable whose values are to be accumulated into the current variable");
  return params;
}

AccumulateAux::AccumulateAux(const std::string & name, InputParameters parameters) :
  AuxKernel(name, parameters),
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
