//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADSmoothTransitionTestMaterial.h"
#include "ADReal.h"

#include "DualReal.h"
registerMooseObject("ThermalHydraulicsTestApp", ADSmoothTransitionTestMaterial);

InputParameters
ADSmoothTransitionTestMaterial::validParams()
{
  InputParameters params = Material::validParams();

  MooseEnum transition_type("cubic weighted");
  params.addRequiredParam<MooseEnum>("transition_type", transition_type, "The transition to test");

  params.addRequiredCoupledVar("var", "Variable on which transition depends");

  params.addClassDescription("Class for testing objects derived from SmoothTransition");

  return params;
}

ADSmoothTransitionTestMaterial::ADSmoothTransitionTestMaterial(const InputParameters & parameters)
  : Material(parameters),

    _transition_type(getParam<MooseEnum>("transition_type")),

    _var(adCoupledValue("var")),
    _center(0.0),
    _width(1.),

    _cubic_transition(_center, _width),
    _weighted_transition(_center, _width),

    _matprop(declareADProperty<Real>("myadmatprop"))
{
  const ADReal & x1 = _cubic_transition.leftEnd();
  const ADReal & x2 = _cubic_transition.rightEnd();
  _cubic_transition.initialize(f1(x1), f2(x2), df1dx(x1), df2dx(x2));
}

void
ADSmoothTransitionTestMaterial::computeQpProperties()
{
  const ADReal & x = _var[_qp];

  if (_transition_type == "cubic")
  {
    _matprop[_qp] = _cubic_transition.value(x, f1(x), f2(x));
  }
  else if (_transition_type == "weighted")
  {
    _matprop[_qp] = _weighted_transition.value(x, f1(x), f2(x));
  }
}

ADReal
ADSmoothTransitionTestMaterial::f1(const ADReal & x) const
{
  return 2 * x + 2;
}

ADReal
ADSmoothTransitionTestMaterial::f2(const ADReal & x) const
{
  return -0.5 * std::pow(x - 1, 2) + 1;
}
ADReal
ADSmoothTransitionTestMaterial::df1dx(const ADReal & /*x*/) const
{
  return 2;
}

ADReal
ADSmoothTransitionTestMaterial::df2dx(const ADReal & x) const
{
  return 1 - x;
}
