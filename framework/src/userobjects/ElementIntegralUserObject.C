//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ElementIntegralUserObject.h"

#include "libmesh/quadrature.h"

InputParameters
ElementIntegralUserObject::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addClassDescription("Performs a spatial integration");
  return params;
}

ElementIntegralUserObject::ElementIntegralUserObject(const InputParameters & parameters)
  : ElementUserObject(parameters), _qp(0), _integral_value(0)
{
}

void
ElementIntegralUserObject::initialize()
{
  _integral_value = 0;
}

void
ElementIntegralUserObject::execute()
{
  _integral_value += computeIntegral();
}

Real
ElementIntegralUserObject::getValue()
{
  gatherSum(_integral_value);
  return _integral_value;
}

void
ElementIntegralUserObject::threadJoin(const UserObject & y)
{
  const ElementIntegralUserObject & pps = static_cast<const ElementIntegralUserObject &>(y);
  _integral_value += pps._integral_value;
}

Real
ElementIntegralUserObject::computeIntegral()
{
  Real sum = 0;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    sum += _JxW[_qp] * _coord[_qp] * computeQpIntegral();
  return sum;
}
