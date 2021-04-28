//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayQuotientAux.h"

registerMooseObject("MooseTestApp", ArrayQuotientAux);

InputParameters
ArrayQuotientAux::validParams()
{
  InputParameters params = ArrayAuxKernel::validParams();
  params.addClassDescription("Divides two coupled variables.");
  params.addCoupledVar("numerator", "The upstairs of the quotient variable");
  params.addCoupledVar("denominator", "The downstairs of the quotient variable");
  return params;
}

ArrayQuotientAux::ArrayQuotientAux(const InputParameters & parameters)
  : ArrayAuxKernel(parameters),
    _numerator(coupledArrayValue("numerator")),
    _denominator(coupledArrayValue("denominator"))
{
}

RealEigenVector
ArrayQuotientAux::computeValue()
{
  return _numerator[_qp].cwiseQuotient(_denominator[_qp]);
}
