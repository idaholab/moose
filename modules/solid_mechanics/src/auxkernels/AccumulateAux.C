//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AccumulateAux.h"

registerMooseObject("SolidMechanicsApp", AccumulateAux);

InputParameters
AccumulateAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Accumulates one or more variables and this auxiliary variable into "
                             "this auxiliary variable");
  params.addRequiredCoupledVar(
      "accumulate_from_variable",
      "Variable whose values are to be accumulated into the current variable");
  return params;
}

AccumulateAux::AccumulateAux(const InputParameters & parameters)
  : AuxKernel(parameters), _values(coupledValues("accumulate_from_variable")), _u_old(uOld())
{
}

Real
AccumulateAux::computeValue()
{
  Real ret = _u_old[_qp];
  for (const auto & value : _values)
    ret += (*value)[_qp];
  return ret;
}
