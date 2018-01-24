//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "QuotientAux.h"

template <>
InputParameters
validParams<QuotientAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addClassDescription("Divides two coupled variables.");
  params.addCoupledVar("numerator", "The upstairs of the quotient variable");
  params.addCoupledVar("denominator", "The downstairs of the quotient variable");
  return params;
}

QuotientAux::QuotientAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _numerator(coupledValue("numerator")),
    _denominator(coupledValue("denominator"))
{
}

Real
QuotientAux::computeValue()
{
  return _numerator[_qp] / _denominator[_qp];
}
