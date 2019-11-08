//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PrimeProductPostprocessor.h"
#include "PrimeProductUserObject.h"

registerMooseObject("MooseTestApp", PrimeProductPostprocessor);

InputParameters
PrimeProductPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<UserObjectName>(
      "prime_product", "The name of the user object that holds the prime product");
  return params;
}

PrimeProductPostprocessor::PrimeProductPostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _prime_product(getUserObject<PrimeProductUserObject>("prime_product"))
{
}

Real
PrimeProductPostprocessor::getValue()
{
  return _prime_product.getProduct();
}
