//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVEnergyHeatCapacityGradient.h"
#include "NS.h"
#include "INSFVRhieChowInterpolator.h"

registerMooseObject("NavierStokesApp", INSFVEnergyHeatCapacityGradient);

InputParameters
INSFVEnergyHeatCapacityGradient::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription("Corrects energy advection for a non-constant specific heat capacity");
  params.addRequiredParam<MooseFunctorName>(
      NS::grad(NS::cp), "The gradient of the constant pressure specific heat capacity");
  params.addRequiredParam<MooseFunctorName>(NS::density, "The density");
  params.addRequiredParam<UserObjectName>("rhie_chow_user_object", "The rhie-chow user-object");
  return params;
}

INSFVEnergyHeatCapacityGradient::INSFVEnergyHeatCapacityGradient(const InputParameters & parameters)
  : FVElementalKernel(parameters),
    _grad_cp(getFunctor<ADRealVectorValue>(NS::grad(NS::cp))),
    _rho(getFunctor<ADReal>(NS::density)),
    _rc_vel_provider(getUserObject<INSFVRhieChowInterpolator>("rhie_chow_user_object"))
{
}

ADReal
INSFVEnergyHeatCapacityGradient::computeQpResidual()
{
  auto elem_arg = makeElemArg(_current_elem);
  const auto state = determineState();
  return -_var(elem_arg, state) * _rho(elem_arg, state) *
         _rc_vel_provider.getVelocity(elem_arg, state, _tid, /*subtract_mesh_velocity=*/true) *
         _grad_cp(elem_arg, state);
}
