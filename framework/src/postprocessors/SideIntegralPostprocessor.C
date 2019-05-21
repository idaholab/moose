//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideIntegralPostprocessor.h"

#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<SideIntegralPostprocessor>()
{
  InputParameters params = validParams<SidePostprocessor>();
  return params;
}

SideIntegralPostprocessor::SideIntegralPostprocessor(const InputParameters & parameters)
  : SidePostprocessor(parameters), _qp(0), _integral_value(0)
{
}

void
SideIntegralPostprocessor::initialize()
{
  _integral_value = 0;
}

void
SideIntegralPostprocessor::execute()
{
  _integral_value += computeIntegral();
}

Real
SideIntegralPostprocessor::getValue()
{
  gatherSum(_integral_value);
  return _integral_value;
}

void
SideIntegralPostprocessor::threadJoin(const UserObject & y)
{
  const SideIntegralPostprocessor & pps = static_cast<const SideIntegralPostprocessor &>(y);
  _integral_value += pps._integral_value;
}

Real
SideIntegralPostprocessor::computeIntegral()
{
  Real sum = 0;
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    sum += _JxW[_qp] * _coord[_qp] * computeQpIntegral();
  return sum;
}
