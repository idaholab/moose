//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThrowMaterial.h"

template <>
InputParameters
validParams<ThrowMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVar("coupled_var", "Name of the coupled variable");
  params.addClassDescription("Test Material that throws MooseExceptions for testing purposes");
  return params;
}

// Must initialize class static variables outside of the class declaration.
bool ThrowMaterial::_has_thrown = false;

ThrowMaterial::ThrowMaterial(const InputParameters & parameters)
  : Material(parameters),
    _prop_value(declareProperty<Real>("matp")),
    _coupled_var(coupledValue("coupled_var"))
{
}

void
ThrowMaterial::residualSetup()
{
  this->comm().max(_has_thrown);
}

void
ThrowMaterial::computeQpProperties()
{
  // 1 + current value squared
  _prop_value[_qp] = 1.0 + _coupled_var[_qp] * _coupled_var[_qp];

  // Throw an exception if we haven't already done so, and the
  // coupled variable has reached a certain value.
  if (_coupled_var[_qp] > 1.0 && !_has_thrown)
  {
    _has_thrown = true;
    throw MooseException("Exception thrown for test purposes.");
  }
}
