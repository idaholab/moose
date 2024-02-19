//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementHCurlError.h"

registerMooseObject("MooseApp", ElementHCurlError);

InputParameters
ElementHCurlError::validParams()
{
  InputParameters params = ElementHCurlSemiError::validParams();
  params.addClassDescription("Returns the H(curl)-norm of the difference between a pair of "
                             "computed and analytical vector-valued solutions.");
  return params;
}

ElementHCurlError::ElementHCurlError(const InputParameters & parameters)
  : ElementHCurlSemiError(parameters)
{
}

Real
ElementHCurlError::computeQpIntegral()
{
  return ElementVectorL2Error::computeQpIntegral() + ElementHCurlSemiError::computeQpIntegral();
}
