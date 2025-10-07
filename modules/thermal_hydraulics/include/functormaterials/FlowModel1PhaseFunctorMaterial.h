//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"
#include "ADUtils.h"
#include "THMNames.h"
#include "SinglePhaseFluidProperties.h"

/**
 * Computes several quantities for FlowModel1Phase.
 */
class FlowModel1PhaseFunctorMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();

  FlowModel1PhaseFunctorMaterial(const InputParameters & parameters);

protected:
  template <bool is_ad>
  void addPressureFunctorProperty();
  template <bool is_ad>
  void addTemperatureFunctorProperty();
  template <bool is_ad>
  void addVelocityFunctorProperty();

  const Moose::Functor<ADReal> & _rhoA;
  const Moose::Functor<ADReal> & _rhouA;
  const Moose::Functor<ADReal> & _rhoEA;
  const Moose::Functor<Real> & _A;

  const SinglePhaseFluidProperties & _fp;
};

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
