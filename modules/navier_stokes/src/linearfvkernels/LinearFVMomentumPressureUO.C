//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVMomentumPressureUO.h"
#include "RhieChowMassFlux.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", LinearFVMomentumPressureUO);

InputParameters
LinearFVMomentumPressureUO::validParams()
{
  InputParameters params = LinearFVElementalKernel::validParams();
  params.addClassDescription(
      "Represents the pressure gradient term in the momentum equation using Rhie-Chow data.");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  params.addRequiredParam<UserObjectName>(
      "rhie_chow_user_object",
      "The rhie-chow user-object which provides the pressure gradient.");
  params.addParam<bool>(
      "use_corrected_gradient",
      true,
      "Whether to use the Rhie-Chow corrected pressure gradient (includes baffle handling).");
  params.addParam<MooseFunctorName>(
      NS::porosity, "1", "Porosity functor (defaults to 1 for non-porous).");
  return params;
}

LinearFVMomentumPressureUO::LinearFVMomentumPressureUO(const InputParameters & params)
  : LinearFVElementalKernel(params),
    _index(getParam<MooseEnum>("momentum_component")),
    _rc_uo(getUserObject<RhieChowMassFlux>("rhie_chow_user_object")),
    _eps(getFunctor<Real>(NS::porosity)),
    _use_corrected_gradient(getParam<bool>("use_corrected_gradient"))
{
}

Real
LinearFVMomentumPressureUO::computeMatrixContribution()
{
  return 0.0;
}

Real
LinearFVMomentumPressureUO::computeRightHandSideContribution()
{
  const Moose::ElemArg elem_arg{_current_elem_info->elem()};
  const auto time_arg = determineState();
  const Real eps = _eps(elem_arg, time_arg);
  const Real grad_p = _use_corrected_gradient
                          ? _rc_uo.pressureGradient(*_current_elem_info, _index)
                          : _rc_uo.rawPressureGradient(*_current_elem_info, _index);
  return -eps * grad_p * _current_elem_volume;
}
