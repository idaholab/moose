//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementH1Error.h"
#include "Function.h"

registerMooseObject("MooseApp", ElementH1Error);

InputParameters
ElementH1Error::validParams()
{
  InputParameters params = ElementW1pError::validParams();
  params.addClassDescription("Computes the H1 error between a variable and a function");
  return params;
}

ElementH1Error::ElementH1Error(const InputParameters & parameters) : ElementW1pError(parameters) {}
