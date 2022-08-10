//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceIntegralPostprocessor.h"

#include "libmesh/quadrature.h"

InputParameters
InterfaceIntegralPostprocessor::validParams()
{
  InputParameters params = InterfacePostprocessor::validParams();
  params.addClassDescription(
      "Postprocessor class adding basic capabilities to compute an integral over an interface. "
      "This class is still abstract, refer to InterfaceIntegralVariableValuePostprocessor for a "
      "derived class example");
  return params;
}

InterfaceIntegralPostprocessor::InterfaceIntegralPostprocessor(const InputParameters & parameters)
  : InterfacePostprocessor(parameters), _qp(0), _integral_value(0)
{
}

void
InterfaceIntegralPostprocessor::initialize()
{
  InterfacePostprocessor::initialize();
  _integral_value = 0;
}

void
InterfaceIntegralPostprocessor::execute()
{
  InterfacePostprocessor::execute();
  _integral_value += computeIntegral();
}

Real
InterfaceIntegralPostprocessor::getValue()
{
  InterfacePostprocessor::getValue();
  return _integral_value;
}

void
InterfaceIntegralPostprocessor::threadJoin(const UserObject & y)
{
  InterfacePostprocessor::threadJoin(y);
  const InterfaceIntegralPostprocessor & pps =
      static_cast<const InterfaceIntegralPostprocessor &>(y);
  _integral_value += pps._integral_value;
}

Real
InterfaceIntegralPostprocessor::computeIntegral()
{
  Real sum = 0;
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    sum += _JxW[_qp] * _coord[_qp] * computeQpIntegral();
  return sum;
}

void
InterfaceIntegralPostprocessor::finalize()
{
  gatherSum(_integral_value);
}
