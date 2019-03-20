//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceIntegralVariableAveragePostprocessor.h"
#include "InterfaceValueTools.h"

registerMooseObject("MooseApp", InterfaceIntegralVariableAveragePostprocessor);

template <>
InputParameters
validParams<InterfaceIntegralVariableAveragePostprocessor>()
{
  InputParameters params = validParams<InterfaceIntegralVariablePostprocessor>();
  params.addParam<MooseEnum>("interface_value_type",
                             InterfaceValueTools::InterfaceAverageOptions(),
                             "Type of average we want to output");
  params.addClassDescription("Computes the average value of a variable on an "
                             "interface. Note that this cannot be used on the "
                             "centerline of an axisymmetric model.");
  return params;
}

InterfaceIntegralVariableAveragePostprocessor::InterfaceIntegralVariableAveragePostprocessor(
    const InputParameters & parameters)
  : InterfaceIntegralVariablePostprocessor(parameters),
    _interface_value_type(parameters.get<MooseEnum>("interface_value_type")),
    _volume(0)
{
}

Real
InterfaceIntegralVariableAveragePostprocessor::computeQpIntegral()
{
  return InterfaceValueTools::getQuantity(_interface_value_type, _u[_qp], _u_neighbor[_qp]);
}

void
InterfaceIntegralVariableAveragePostprocessor::initialize()
{
  InterfaceIntegralVariablePostprocessor::initialize();
  _volume = 0;
}

void
InterfaceIntegralVariableAveragePostprocessor::execute()
{
  InterfaceIntegralVariablePostprocessor::execute();
  _volume += _current_side_volume;
}

Real
InterfaceIntegralVariableAveragePostprocessor::getValue()
{
  Real integral = InterfaceIntegralVariablePostprocessor::getValue();
  gatherSum(_volume);
  return integral / _volume;
}

void
InterfaceIntegralVariableAveragePostprocessor::threadJoin(const UserObject & y)
{
  InterfaceIntegralVariablePostprocessor::threadJoin(y);
  const InterfaceIntegralVariableAveragePostprocessor & pps =
      static_cast<const InterfaceIntegralVariableAveragePostprocessor &>(y);
  _volume += pps._volume;
}
