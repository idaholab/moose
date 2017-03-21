/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
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
