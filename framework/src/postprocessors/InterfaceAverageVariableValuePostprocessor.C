//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceAverageVariableValuePostprocessor.h"
#include "InterfaceValueTools.h"

registerMooseObject("MooseApp", InterfaceAverageVariableValuePostprocessor);

InputParameters
InterfaceAverageVariableValuePostprocessor::validParams()
{
  InputParameters params = InterfaceIntegralVariableValuePostprocessor::validParams();
  params.addClassDescription("Computes the average value of a variable on an "
                             "interface. Note that this cannot be used on the "
                             "centerline of an axisymmetric model.");
  return params;
}

InterfaceAverageVariableValuePostprocessor::InterfaceAverageVariableValuePostprocessor(
    const InputParameters & parameters)
  : InterfaceIntegralVariableValuePostprocessor(parameters)
{
}

Real
InterfaceAverageVariableValuePostprocessor::getValue() const
{
  Real integral = InterfaceIntegralVariableValuePostprocessor::getValue();
  return integral / _interface_primary_area;
}
