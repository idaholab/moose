//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMNormalizedVariableStep.h"

registerMooseObject("ThermalHydraulicsApp", THMNormalizedVariableStep);

InputParameters
THMNormalizedVariableStep::validParams()
{
  InputParameters params = ElementExtremeFunctorValue::validParams();

  params.addClassDescription("Computes a normalized variable step norm for various variables.");

  params.addRequiredParam<Real>("normalization", "Normalization constant");

  return params;
}

THMNormalizedVariableStep::THMNormalizedVariableStep(const InputParameters & parameters)
  : ElementExtremeFunctorValue(parameters), _normalization(getParam<Real>("normalization"))
{
}

Real
THMNormalizedVariableStep::getValue() const
{
  return ElementExtremeFunctorValue::getValue() / _normalization;
}
