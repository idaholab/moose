//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVMomentumFriction.h"
#include "NS.h"
#include "NavierStokesMethods.h"

registerMooseObject("NavierStokesApp", LinearFVMomentumFriction);

InputParameters
LinearFVMomentumFriction::validParams()
{
  InputParameters params = LinearFVElementalKernel::validParams();
  params.addClassDescription(
      "Computes Darcy and/or Forchheimer resistance in a Navier-Stokes momentum equation.");
  params.addParam<MooseFunctorName>("Darcy_name", "Name of the Darcy coefficients property.");
  params.addParam<MooseFunctorName>("Forchheimer_name",
                                    "Name of the Forchheimer coefficients property.");
  params.addParam<MooseFunctorName>(NS::mu, "The dynamic viscosity required by Darcy resistance.");
  params.addParam<MooseFunctorName>(NS::density, "The density required by Forchheimer resistance.");
  params.addParam<MooseFunctorName>(NS::porosity, "1", "The porosity.");
  params.addParam<MooseFunctorName>("u", "The x-component of superficial velocity.");
  params.addParam<MooseFunctorName>("v", "The y-component of superficial velocity.");
  params.addParam<MooseFunctorName>("w", "The z-component of superficial velocity.");

  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");

  return params;
}

LinearFVMomentumFriction::LinearFVMomentumFriction(const InputParameters & params)
  : LinearFVElementalKernel(params),
    _index(getParam<MooseEnum>("momentum_component")),
    _D(isParamValid("Darcy_name") ? &getFunctor<RealVectorValue>("Darcy_name") : nullptr),
    _F(isParamValid("Forchheimer_name") ? &getFunctor<RealVectorValue>("Forchheimer_name")
                                        : nullptr),
    _mu(isParamValid(NS::mu) ? &getFunctor<Real>(NS::mu) : nullptr),
    _rho(isParamValid(NS::density) ? &getFunctor<Real>(NS::density) : nullptr),
    _porosity(getFunctor<Real>(NS::porosity)),
    _dim(_subproblem.mesh().dimension()),
    _u(isParamValid("u") ? &getFunctor<Real>("u") : nullptr),
    _v(isParamValid("v") ? &getFunctor<Real>("v") : nullptr),
    _w(isParamValid("w") ? &getFunctor<Real>("w") : nullptr)
{
  if (!_D && !_F)
    paramError("Darcy_name", "Provide Darcy_name, Forchheimer_name, or both.");
  if (_D && !_mu)
    paramError(NS::mu, "The mu parameter is required when Darcy_name is provided.");
  if (_F && !_rho)
    paramError(NS::density, "The rho parameter is required when Forchheimer_name is provided.");
  if (_F && !_u)
    paramError("u", "The u parameter is required when Forchheimer_name is provided.");
  if (_F && _dim >= 2 && !_v)
    paramError("v", "The v parameter is required for Forchheimer resistance in 2D or 3D.");
  if (_F && _dim >= 3 && !_w)
    paramError("w", "The w parameter is required for Forchheimer resistance in 3D.");
}

Real
LinearFVMomentumFriction::computeMatrixContribution()
{
  const auto elem_arg = makeElemArg(_current_elem_info->elem());
  const auto state = determineState();
  return computeFrictionCoefficient(elem_arg, state) * _current_elem_volume;
}

Real
LinearFVMomentumFriction::computeRightHandSideContribution()
{
  return 0.0;
}

Real
LinearFVMomentumFriction::computeFrictionCoefficient(const Moose::ElemArg & elem_arg,
                                                     const Moose::StateArg & state) const
{
  Real coefficient = _D ? (*_mu)(elem_arg, state) * (*_D)(elem_arg, state)(_index) : 0.0;

  if (_F)
  {
    const Real porosity = _porosity(elem_arg, state);
    if (porosity <= 0.0)
      mooseError(name(), ": porosity must be positive when using Forchheimer friction.");

    const Real u = (*_u)(elem_arg, state);
    const Real v = _v ? (*_v)(elem_arg, state) : 0.0;
    const Real w = _w ? (*_w)(elem_arg, state) : 0.0;
    const Real speed = NS::computeSpeed(RealVectorValue(u, v, w)) / porosity;
    coefficient += 0.5 * (*_rho)(elem_arg, state) * (*_F)(elem_arg, state)(_index)*speed;
  }

  return coefficient;
}
