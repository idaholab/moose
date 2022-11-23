//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementIntegralPostprocessor.h"

#include "libmesh/quadrature.h"

InputParameters
ElementIntegralPostprocessor::validParams()
{
  InputParameters params = ElementPostprocessor::validParams();
  return params;
}

ElementIntegralPostprocessor::ElementIntegralPostprocessor(const InputParameters & parameters)
  : ElementPostprocessor(parameters), _qp(0), _integral_value(0)
{
}

void
ElementIntegralPostprocessor::initialize()
{
  _integral_value = 0;
}

void
ElementIntegralPostprocessor::execute()
{
  _integral_value += computeIntegral();
}

Real
ElementIntegralPostprocessor::getValue()
{
  return _integral_value;
}

void
ElementIntegralPostprocessor::threadJoin(const UserObject & y)
{
  const ElementIntegralPostprocessor & pps = static_cast<const ElementIntegralPostprocessor &>(y);
  _integral_value += pps._integral_value;
}

Real
ElementIntegralPostprocessor::computeIntegral()
{
  Real sum = 0;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    sum += _JxW[_qp] * _coord[_qp] * computeQpIntegral();
  return sum;
}

void
ElementIntegralPostprocessor::finalize()
{
  gatherSum(_integral_value);
}
