//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionElementLoopIntegralUserObject.h"
#include "Assembly.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", FunctionElementLoopIntegralUserObject);

InputParameters
FunctionElementLoopIntegralUserObject::validParams()
{
  InputParameters params = ElementLoopUserObject::validParams();

  params.addClassDescription("Computes the integral of a function using an element loop.");

  params.addRequiredParam<FunctionName>("function", "Function to integrate.");

  return params;
}

FunctionElementLoopIntegralUserObject::FunctionElementLoopIntegralUserObject(
    const InputParameters & parameters)
  : ElementLoopUserObject(parameters),

    _function(getFunction("function")),
    _qp(0),
    _integral_value(0)
{
}

void
FunctionElementLoopIntegralUserObject::initialize()
{
  _integral_value = 0;
}

void
FunctionElementLoopIntegralUserObject::computeElement()
{
  _assembly.reinit(_current_elem);
  _integral_value += computeIntegral();
}

void
FunctionElementLoopIntegralUserObject::threadJoin(const UserObject & y)
{
  const FunctionElementLoopIntegralUserObject & uo =
      static_cast<const FunctionElementLoopIntegralUserObject &>(y);
  _integral_value += uo._integral_value;
}

void
FunctionElementLoopIntegralUserObject::finalize()
{
  gatherSum(_integral_value);
}

Real
FunctionElementLoopIntegralUserObject::computeIntegral()
{
  Real sum = 0;
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    sum += _JxW[_qp] * _coord[_qp] * computeQpIntegral();
  return sum;
}

Real
FunctionElementLoopIntegralUserObject::computeQpIntegral()
{
  return _function.value(_t, _q_point[_qp]);
}

Real
FunctionElementLoopIntegralUserObject::getValue() const
{
  return _integral_value;
}
