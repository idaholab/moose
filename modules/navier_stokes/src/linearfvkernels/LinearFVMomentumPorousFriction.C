//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVMomentumPorousFriction.h"
#include "NS.h"
#include "NavierStokesMethods.h"

registerMooseObject("NavierStokesApp", LinearFVMomentumPorousFriction);

InputParameters
LinearFVMomentumPorousFriction::validParams()
{
  InputParameters params = LinearFVElementalKernel::validParams();
  params.addClassDescription(
      "Computes Darcy and/or Forchheimer porous resistance on the Navier Stokes i-th momentum "
      "equation using interstitial velocity.");

  params.addParam<MooseFunctorName>("Darcy_name", "Name of the Darcy coefficients property.");
  params.addParam<MooseFunctorName>("Forchheimer_name",
                                    "Name of the Forchheimer coefficients property.");
  params.addParam<MooseFunctorName>(
      NS::porosity, "1", "Porosity functor (defaults to 1 for non-porous).");
  params.addParam<MooseFunctorName>(NS::mu, "The dynamic viscosity (required for Darcy).");
  params.addParam<MooseFunctorName>(NS::density, "The density (required for Forchheimer).");

  params.addParam<MooseFunctorName>("u", "The x-component of velocity");
  params.addParam<MooseFunctorName>("v", "The y-component of velocity");
  params.addParam<MooseFunctorName>("w", "The z-component of velocity");

  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");

  return params;
}

LinearFVMomentumPorousFriction::LinearFVMomentumPorousFriction(const InputParameters & params)
  : LinearFVElementalKernel(params),
    _index(getParam<MooseEnum>("momentum_component")),
    _dim(_subproblem.mesh().dimension()),
    _darcy(isParamValid("Darcy_name") ? &getFunctor<RealVectorValue>("Darcy_name") : nullptr),
    _forchheimer(isParamValid("Forchheimer_name") ? &getFunctor<RealVectorValue>("Forchheimer_name")
                                                  : nullptr),
    _eps(getFunctor<Real>(NS::porosity)),
    _mu(isParamValid(NS::mu) ? &getFunctor<Real>(NS::mu) : nullptr),
    _rho(isParamValid(NS::density) ? &getFunctor<Real>(NS::density) : nullptr),
    _u(isParamValid("u") ? &getFunctor<Real>("u") : nullptr),
    _v(isParamValid("v") ? &getFunctor<Real>("v") : nullptr),
    _w(isParamValid("w") ? &getFunctor<Real>("w") : nullptr)
{
  if (!_darcy && !_forchheimer)
    paramError("Darcy_name", "Provide Darcy_name, Forchheimer_name, or both.");

  if (_darcy && !_mu)
    paramError(NS::mu, "The mu parameter must be supplied when Darcy_name is provided.");

  if (_forchheimer)
  {
    if (!_rho)
      paramError(NS::density,
                 "The density parameter must be supplied when Forchheimer_name is provided.");
    if (!_u)
      paramError("u", "The u parameter must be supplied when Forchheimer_name is provided.");
    if (_dim >= 2 && !_v)
      paramError("v",
                 "The v parameter must be supplied when Forchheimer_name is used in 2D or 3D.");
    if (_dim >= 3 && !_w)
      paramError("w", "The w parameter must be supplied when Forchheimer_name is used in 3D.");
  }
}

Real
LinearFVMomentumPorousFriction::computeMatrixContribution()
{
  const auto elem_arg = makeElemArg(_current_elem_info->elem());
  const auto state = determineState();
  const Real coeff =
      computeDarcyCoefficient(elem_arg, state) + computeForchheimerCoefficient(elem_arg, state);
  return coeff * _current_elem_volume;
}

Real
LinearFVMomentumPorousFriction::computeRightHandSideContribution()
{
  return 0.0;
}

Real
LinearFVMomentumPorousFriction::computeDarcyCoefficient(const Moose::ElemArg & elem_arg,
                                                        const Moose::StateArg & state) const
{
  if (!_darcy)
    return 0.0;

  const Real eps = _eps(elem_arg, state);
  mooseAssert(eps > 0.0, "Epsilon must be greater than 0!");

  return (*_mu)(elem_arg, state) * (*_darcy)(elem_arg, state)(_index) / eps;
}

Real
LinearFVMomentumPorousFriction::computeForchheimerCoefficient(const Moose::ElemArg & elem_arg,
                                                              const Moose::StateArg & state) const
{
  if (!_forchheimer)
    return 0.0;

  const Real eps = _eps(elem_arg, state);
  mooseAssert(eps > 0.0, "Epsilon must be greater than 0!");

  const Real u = (*_u)(elem_arg, state);
  const Real v = _v ? (*_v)(elem_arg, state) : 0.0;
  const Real w = _w ? (*_w)(elem_arg, state) : 0.0;
  const Real speed = NS::computeSpeed(RealVectorValue(u, v, w)) / eps;

  return (*_rho)(elem_arg, state) * (*_forchheimer)(elem_arg, state)(_index)*speed / eps;
}
