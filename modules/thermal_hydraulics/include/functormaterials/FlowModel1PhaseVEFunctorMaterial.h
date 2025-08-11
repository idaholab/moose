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

class SinglePhaseFluidProperties;

/**
 * Computes several quantities for FlowModel1Phase.
 */
class FlowModel1PhaseVEFunctorMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();

  FlowModel1PhaseVEFunctorMaterial(const InputParameters & parameters);

protected:
  template <bool is_ad>
  void addSpecificVolumeFunctorProperty();
  template <bool is_ad>
  void addSpecificInternalEnergyFunctorProperty();

  const Moose::Functor<ADReal> & _rhoA;
  const Moose::Functor<ADReal> & _rhouA;
  const Moose::Functor<ADReal> & _rhoEA;
  const Moose::Functor<Real> & _A;
};

template <bool is_ad>
void
FlowModel1PhaseVEFunctorMaterial::addSpecificVolumeFunctorProperty()
{
  addFunctorProperty<GenericReal<is_ad>>(
      THM::functorMaterialPropertyName<is_ad>(THM::SPECIFIC_VOLUME),
      [this](const auto & r, const auto & t) -> GenericReal<is_ad>
      {
        const auto rhoA = Moose::ADRealToGenericReal<is_ad>(_rhoA(r, t));
        const auto A = _A(r, t);
        return A / rhoA;
      });
}

template <bool is_ad>
void
FlowModel1PhaseVEFunctorMaterial::addSpecificInternalEnergyFunctorProperty()
{
  addFunctorProperty<GenericReal<is_ad>>(
      THM::functorMaterialPropertyName<is_ad>(THM::SPECIFIC_INTERNAL_ENERGY),
      [this](const auto & r, const auto & t) -> GenericReal<is_ad>
      {
        const auto rhoA = Moose::ADRealToGenericReal<is_ad>(_rhoA(r, t));
        const auto rhouA = Moose::ADRealToGenericReal<is_ad>(_rhouA(r, t));
        const auto rhoEA = Moose::ADRealToGenericReal<is_ad>(_rhoEA(r, t));
        return (rhoEA - 0.5 * rhouA * rhouA / rhoA) / rhoA;
      });
}
