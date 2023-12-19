//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementHDivError.h"

registerMooseObject("MooseApp", ElementHDivError);

InputParameters
ElementHDivError::validParams()
{
  InputParameters params = ElementHDivSemiError::validParams();
  params.addClassDescription("Returns the H(div)-norm of the difference between a pair of "
                             "computed and analytical vector-valued solutions.");
  return params;
}

ElementHDivError::ElementHDivError(const InputParameters & parameters)
  : ElementHDivSemiError(parameters)
{
}

Real
ElementHDivError::computeQpIntegral()
{
  return ElementVectorL2Error::computeQpIntegral() + ElementHDivSemiError::computeQpIntegral();
}
