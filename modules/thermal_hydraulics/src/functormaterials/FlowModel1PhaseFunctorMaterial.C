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
    _rhoEA(getFunctor<ADReal>(THM::RHOEA)),
    _A(getFunctor<Real>(THM::AREA)),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
  // pressure
  addPressureFunctorProperty<false>();

  // temperature
  addTemperatureFunctorProperty<false>();

  // velocity
  addVelocityFunctorProperty<false>();
}

template <bool is_ad>
void
FlowModel1PhaseFunctorMaterial::addPressureFunctorProperty()
{
  addFunctorProperty<GenericReal<is_ad>>(
      THM::functorMaterialPropertyName<is_ad>(THM::PRESSURE),
      [this](const auto & r, const auto & t) -> GenericReal<is_ad>
      {
        const auto rhoA = Moose::ADRealToGenericReal<is_ad>(_rhoA(r, t));
        const auto rhouA = Moose::ADRealToGenericReal<is_ad>(_rhouA(r, t));
        const auto rhoEA = Moose::ADRealToGenericReal<is_ad>(_rhoEA(r, t));
        const auto A = _A(r, t);
        const auto v = A / rhoA;
        const auto e = (rhoEA - 0.5 * rhouA * rhouA / rhoA) / rhoA;
        return _fp.p_from_v_e(v, e);
      });
}

template <bool is_ad>
void
FlowModel1PhaseFunctorMaterial::addTemperatureFunctorProperty()
{
  addFunctorProperty<GenericReal<is_ad>>(
      THM::functorMaterialPropertyName<is_ad>(THM::TEMPERATURE),
      [this](const auto & r, const auto & t) -> GenericReal<is_ad>
      {
        const auto rhoA = Moose::ADRealToGenericReal<is_ad>(_rhoA(r, t));
        const auto rhouA = Moose::ADRealToGenericReal<is_ad>(_rhouA(r, t));
        const auto rhoEA = Moose::ADRealToGenericReal<is_ad>(_rhoEA(r, t));
        const auto A = _A(r, t);
        const auto v = A / rhoA;
        const auto e = (rhoEA - 0.5 * rhouA * rhouA / rhoA) / rhoA;
        return _fp.T_from_v_e(v, e);
      });
}

template <bool is_ad>
void
FlowModel1PhaseFunctorMaterial::addVelocityFunctorProperty()
{
  addFunctorProperty<GenericReal<is_ad>>(
      THM::functorMaterialPropertyName<is_ad>(THM::VELOCITY),
      [this](const auto & r, const auto & t) -> GenericReal<is_ad>
      {
        const auto rhoA = Moose::ADRealToGenericReal<is_ad>(_rhoA(r, t));
        const auto rhouA = Moose::ADRealToGenericReal<is_ad>(_rhouA(r, t));
        return rhouA / rhoA;
      });
}
