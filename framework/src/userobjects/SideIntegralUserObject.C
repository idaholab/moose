//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideIntegralUserObject.h"

#include "libmesh/quadrature.h"

InputParameters
SideIntegralUserObject::validParams()
{
  InputParameters params = SideUserObject::validParams();
  return params;
}

SideIntegralUserObject::SideIntegralUserObject(const InputParameters & parameters)
  : SideUserObject(parameters), _qp(0), _integral_value(0)
{
}

void
SideIntegralUserObject::initialize()
{
  _integral_value = 0;
}

void
SideIntegralUserObject::execute()
{
  _integral_value += computeIntegral();
}

Real
SideIntegralUserObject::getValue()
{
  gatherSum(_integral_value);
  return _integral_value;
}

void
SideIntegralUserObject::threadJoin(const UserObject & y)
{
  const SideIntegralUserObject & pps = static_cast<const SideIntegralUserObject &>(y);
  _integral_value += pps._integral_value;
}

Real
SideIntegralUserObject::computeIntegral()
{
  Real sum = 0;
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    sum += _JxW[_qp] * _coord[_qp] * computeQpIntegral();
  return sum;
}
