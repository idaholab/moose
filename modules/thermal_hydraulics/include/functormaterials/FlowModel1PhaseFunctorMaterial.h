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

  template <bool is_ad>
  const Moose::Functor<GenericReal<is_ad>> &
  getGenericRealFunctor(const Moose::Functor<ADReal> & ad_functor,
                        const Moose::Functor<Real> & nonad_functor) const;

  const Moose::Functor<ADReal> & _rhoA;
  const Moose::Functor<ADReal> & _rhouA;

  const Moose::Functor<ADReal> & _v_ad;
  const Moose::Functor<ADReal> & _e_ad;

  const Moose::Functor<Real> & _v_nonad;
  const Moose::Functor<Real> & _e_nonad;

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
        const auto v = getGenericRealFunctor<is_ad>(_v_ad, _v_nonad)(r, t);
        const auto e = getGenericRealFunctor<is_ad>(_e_ad, _e_nonad)(r, t);
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
        const auto v = getGenericRealFunctor<is_ad>(_v_ad, _v_nonad)(r, t);
        const auto e = getGenericRealFunctor<is_ad>(_e_ad, _e_nonad)(r, t);
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

template <bool is_ad>
const Moose::Functor<GenericReal<is_ad>> &
FlowModel1PhaseFunctorMaterial::getGenericRealFunctor(
    const Moose::Functor<ADReal> & ad_functor, const Moose::Functor<Real> & nonad_functor) const
{
  if constexpr (is_ad)
    return ad_functor;
  else
    return nonad_functor;
}
