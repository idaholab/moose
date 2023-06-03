//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowDarcyVelocityComponent.h"

registerMooseObject("PorousFlowApp", PorousFlowDarcyVelocityComponent);
registerMooseObject("PorousFlowApp", ADPorousFlowDarcyVelocityComponent);

template <bool is_ad>
InputParameters
PorousFlowDarcyVelocityComponentTempl<is_ad>::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredParam<RealVectorValue>("gravity",
                                           "Gravitational acceleration vector downwards (m/s^2)");
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names");
  params.addParam<unsigned int>("fluid_phase", 0, "The index corresponding to the fluid phase");
  MooseEnum component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "component", component, "The spatial component of the Darcy flux to return");
  params.addClassDescription("Darcy velocity (in m^3.s^-1.m^-2, or m.s^-1)  -(k_ij * krel /mu "
                             "(nabla_j P - w_j)), where k_ij is the permeability tensor, krel is "
                             "the relative permeability, mu is the fluid viscosity, P is the fluid "
                             "pressure, and w_j is the fluid weight.");
  return params;
}

template <bool is_ad>
PorousFlowDarcyVelocityComponentTempl<is_ad>::PorousFlowDarcyVelocityComponentTempl(
    const InputParameters & parameters)
  : AuxKernel(parameters),
    _relative_permeability(getGenericMaterialProperty<std::vector<Real>, is_ad>(
        "PorousFlow_relative_permeability_qp")),
    _fluid_viscosity(
        getGenericMaterialProperty<std::vector<Real>, is_ad>("PorousFlow_viscosity_qp")),
    _permeability(getGenericMaterialProperty<RealTensorValue, is_ad>("PorousFlow_permeability_qp")),
    _grad_p(getGenericMaterialProperty<std::vector<RealGradient>, is_ad>(
        "PorousFlow_grad_porepressure_qp")),
    _fluid_density_qp(
        getGenericMaterialProperty<std::vector<Real>, is_ad>("PorousFlow_fluid_phase_density_qp")),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _ph(getParam<unsigned int>("fluid_phase")),
    _component(getParam<MooseEnum>("component")),
    _gravity(getParam<RealVectorValue>("gravity"))
{
  if (_ph >= _dictator.numPhases())
    paramError("fluid_phase",
               "The Dictator proclaims that the maximum phase index in this simulation is ",
               _dictator.numPhases() - 1,
               " whereas you have used ",
               _ph,
               ". Remember that indexing starts at 0. The Dictator is watching you, to "
               "ensure your wellbeing.");
}

template <bool is_ad>
Real
PorousFlowDarcyVelocityComponentTempl<is_ad>::computeValue()
{
  return -MetaPhysicL::raw_value(
      (_permeability[_qp] * (_grad_p[_qp][_ph] - _fluid_density_qp[_qp][_ph] * _gravity) *
       _relative_permeability[_qp][_ph] / _fluid_viscosity[_qp][_ph])(_component));
}

template class PorousFlowDarcyVelocityComponentTempl<false>;
template class PorousFlowDarcyVelocityComponentTempl<true>;
