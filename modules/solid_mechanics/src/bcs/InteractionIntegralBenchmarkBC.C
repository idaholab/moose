//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InteractionIntegralBenchmarkBC.h"
#include "Function.h"

registerMooseObject("TensorMechanicsApp", InteractionIntegralBenchmarkBC);

InputParameters
InteractionIntegralBenchmarkBC::validParams()
{
  MooseEnum disp_component("x=0 y=1 z=2");
  InputParameters params = DirichletBCBase::validParams();
  params.addClassDescription("Implements a boundary condition that enforces a displacement field "
                             "around a crack tip based on applied stress intensity factors.");
  params.addRequiredParam<MooseEnum>(
      "component", disp_component, "The component of the displacement to apply BC on.");
  params.addRequiredParam<UserObjectName>("crack_front_definition",
                                          "The CrackFrontDefinition user object name");
  params.addParam<unsigned int>(
      "crack_front_point_index", 0, "The index of the point on the crack front.");
  params.addRequiredParam<Real>("poissons_ratio", "Poisson's ratio for the material.");
  params.addRequiredParam<Real>("youngs_modulus", "Young's modulus of the material.");
  params.addRequiredParam<FunctionName>("KI_function",
                                        "Function describing the Mode I stress intensity factor.");
  params.addRequiredParam<FunctionName>("KII_function",
                                        "Function describing the Mode II stress intensity factor.");
  params.addRequiredParam<FunctionName>(
      "KIII_function", "Function describing the Mode III stress intensity factor.");
  params.set<bool>("preset") = true;
  return params;
}

InteractionIntegralBenchmarkBC::InteractionIntegralBenchmarkBC(const InputParameters & parameters)
  : DirichletBCBase(parameters),
    _component(getParam<MooseEnum>("component")),
    _crack_front_definition(getUserObject<CrackFrontDefinition>("crack_front_definition")),
    _crack_front_point_index(getParam<unsigned int>("crack_front_point_index")),
    _poissons_ratio(getParam<Real>("poissons_ratio")),
    _youngs_modulus(getParam<Real>("youngs_modulus")),
    _ki_function(getFunction("KI_function")),
    _kii_function(getFunction("KII_function")),
    _kiii_function(getFunction("KIII_function"))
{
  _kappa = 3 - 4 * _poissons_ratio;
  _mu = _youngs_modulus / (2 * (1 + _poissons_ratio));
}

Real
InteractionIntegralBenchmarkBC::computeQpValue()
{
  const Real ki_val = _ki_function.value(_t, *_current_node);
  const Real kii_val = _kii_function.value(_t, *_current_node);
  const Real kiii_val = _kiii_function.value(_t, *_current_node);

  const Point p(*_current_node);
  _crack_front_definition.calculateRThetaToCrackFront(p, _crack_front_point_index, _r, _theta);

  if (_r == 0)
    _theta = 0;

  const Real st2 = std::sin(_theta / 2.0);
  const Real ct2 = std::cos(_theta / 2.0);

  Real disp(0.0);

  if (_component == 0)
    disp = 1 / (2 * _mu) * std::sqrt(_r / (2 * libMesh::pi)) *
           (ki_val * ct2 * (_kappa - 1 + 2 * st2 * st2) +
            kii_val * st2 * (_kappa + 1 + 2 * ct2 * ct2));
  else if (_component == 1)
    disp = 1 / (2 * _mu) * std::sqrt(_r / (2 * libMesh::pi)) *
           (ki_val * st2 * (_kappa + 1 - 2 * ct2 * ct2) -
            kii_val * ct2 * (_kappa - 1 - 2 * st2 * st2));
  else if (_component == 2)
    disp = 1 / _mu * std::sqrt(2 * _r / libMesh::pi) * kiii_val * st2;

  return disp;
}
