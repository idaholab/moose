//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "QuotientScalarAux.h"

registerMooseObject("MooseApp", QuotientScalarAux);

InputParameters
QuotientScalarAux::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();
  params.addClassDescription("Compute the ratio of two scalar variables.");
  params.addCoupledVar("numerator", "The upstairs of the quotient variable");
  params.addCoupledVar("denominator", "The downstairs of the quotient variable");

  return params;
}

QuotientScalarAux::QuotientScalarAux(const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    _a(coupledScalarValue("numerator")),
    _b(coupledScalarValue("denominator"))
{
}

Real
QuotientScalarAux::computeValue()
{
  return _a[0] / _b[0];
}
