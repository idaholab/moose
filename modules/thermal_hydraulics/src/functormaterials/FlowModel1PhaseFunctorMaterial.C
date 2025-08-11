//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowModel1PhaseFunctorMaterial.h"

registerMooseObject("ThermalHydraulicsApp", FlowModel1PhaseFunctorMaterial);

InputParameters
FlowModel1PhaseFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();

  params.addClassDescription("Computes several quantities for FlowModel1Phase.");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "The SinglePhaseFluidProperties object for the fluid");

  return params;
}

FlowModel1PhaseFunctorMaterial::FlowModel1PhaseFunctorMaterial(const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _rhoA(getFunctor<ADReal>(THM::RHOA)),
    _rhouA(getFunctor<ADReal>(THM::RHOUA)),
    _v_ad(getFunctor<ADReal>(THM::functorMaterialPropertyName<true>(THM::SPECIFIC_VOLUME))),
    _e_ad(
        getFunctor<ADReal>(THM::functorMaterialPropertyName<true>(THM::SPECIFIC_INTERNAL_ENERGY))),
    _v_nonad(getFunctor<Real>(THM::functorMaterialPropertyName<false>(THM::SPECIFIC_VOLUME))),
    _e_nonad(
        getFunctor<Real>(THM::functorMaterialPropertyName<false>(THM::SPECIFIC_INTERNAL_ENERGY))),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
  // pressure
  addPressureFunctorProperty<true>();
  addPressureFunctorProperty<false>();

  // temperature
  addTemperatureFunctorProperty<true>();
  addTemperatureFunctorProperty<false>();

  // velocity
  addVelocityFunctorProperty<true>();
  addVelocityFunctorProperty<false>();
}
