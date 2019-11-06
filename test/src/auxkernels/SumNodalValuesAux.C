//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SumNodalValuesAux.h"

registerMooseObject("MooseTestApp", SumNodalValuesAux);

InputParameters
SumNodalValuesAux::validParams()
{
  InputParameters params = AuxNodalScalarKernel::validParams();
  params.addRequiredCoupledVar("sum_var", "Variable to be summed");

  return params;
}

SumNodalValuesAux::SumNodalValuesAux(const InputParameters & parameters)
  : AuxNodalScalarKernel(parameters), _sum_var(coupledValue("sum_var"))
{
}

SumNodalValuesAux::~SumNodalValuesAux() {}

void
SumNodalValuesAux::compute()
{
  _subproblem.reinitNodes(_node_ids, _tid); // compute variables at nodes
  for (_i = 0; _i < _var.order(); ++_i)
  {
    Real value = computeValue();
    _communicator.sum(value);
    _var.setValue(_i, value); // update variable data, which is referenced by other kernels, so the
                              // value is up-to-date
  }
}

Real
SumNodalValuesAux::computeValue()
{
  Real sum = 0;
  for (unsigned int i = 0; i < _sum_var.size(); i++)
    sum += _sum_var[i];
  return sum;
}
