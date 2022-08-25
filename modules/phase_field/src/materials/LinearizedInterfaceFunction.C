//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearizedInterfaceFunction.h"

registerMooseObject("PhaseFieldApp", LinearizedInterfaceFunction);

InputParameters
LinearizedInterfaceFunction::validParams()
{
  InputParameters params = DerivativeParsedMaterialHelper::validParams();
  params.addClassDescription(
      "Defines the order parameter substitution for linearized interface phase field models");
  params.addRequiredCoupledVar("phi", "Concentration variable");
  return params;
}

LinearizedInterfaceFunction::LinearizedInterfaceFunction(const InputParameters & parameters)
  : DerivativeParsedMaterialHelper(parameters), _phi("phi")
{
  EBFunction order_parameter;
  // Definition of the function for the expression builder
  order_parameter(_phi) = 1.0 / 2.0 * (1.0 + tanh(_phi / sqrt(2.0)));

  // Parse function for automatic differentiation
  functionParse(order_parameter);
}
