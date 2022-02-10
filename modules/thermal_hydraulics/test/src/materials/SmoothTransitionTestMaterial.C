//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SmoothTransitionTestMaterial.h"

registerMooseObject("ThermalHydraulicsTestApp", SmoothTransitionTestMaterial);

InputParameters
SmoothTransitionTestMaterial::validParams()
{
  InputParameters params = Material::validParams();

  MooseEnum transition_type("cubic weighted");
  params.addRequiredParam<MooseEnum>("transition_type", transition_type, "The transition to test");

  params.addRequiredCoupledVar("var", "Variable on which transition depends");

  params.addClassDescription("Class for testing objects derived from SmoothTransition");

  return params;
}

SmoothTransitionTestMaterial::SmoothTransitionTestMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Material>(parameters),

    _transition_type(getParam<MooseEnum>("transition_type")),

    _var(coupledValue("var")),

    _cubic_transition(0.0, 1.0),
    _weighted_transition(0.0, 1.0),

    _matprop(declareProperty<Real>("mymatprop")),
    _dmatprop_dvar(declarePropertyDerivativeTHM<Real>("mymatprop", "var"))
{
  const Real & x1 = _cubic_transition.leftEnd();
  const Real & x2 = _cubic_transition.rightEnd();
  _cubic_transition.initialize(f1(x1), f2(x2), df1dx(x1), df2dx(x2));
}

void
SmoothTransitionTestMaterial::computeQpProperties()
{
  const Real & x = _var[_qp];

  if (_transition_type == "cubic")
  {
    _matprop[_qp] = _cubic_transition.value(x, f1(x), f2(x));
    _dmatprop_dvar[_qp] = _cubic_transition.derivative(x, df1dx(x), df2dx(x));
  }
  else if (_transition_type == "weighted")
  {
    _matprop[_qp] = _weighted_transition.value(x, f1(x), f2(x));
    _dmatprop_dvar[_qp] = _weighted_transition.derivative(x, f1(x), f2(x), df1dx(x), df2dx(x));
  }
}

Real
SmoothTransitionTestMaterial::f1(const Real & x) const
{
  return 2 * x + 2;
}

Real
SmoothTransitionTestMaterial::f2(const Real & x) const
{
  return -0.5 * std::pow(x - 1, 2) + 1;
}

Real
SmoothTransitionTestMaterial::df1dx(const Real & /*x*/) const
{
  return 2;
}

Real
SmoothTransitionTestMaterial::df2dx(const Real & x) const
{
  return 1 - x;
}
