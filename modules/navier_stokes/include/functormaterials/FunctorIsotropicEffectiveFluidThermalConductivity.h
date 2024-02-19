//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorEffectiveFluidThermalConductivity.h"
#include "NS.h"

/**
 * Material providing an isotropic effective fluid thermal conductivity.
 */
template <typename Derived>
class FunctorIsotropicEffectiveFluidThermalConductivity
  : public FunctorEffectiveFluidThermalConductivity
{
public:
  FunctorIsotropicEffectiveFluidThermalConductivity(const InputParameters & parameters);

  static InputParameters validParams();
};

template <typename Derived>
InputParameters
FunctorIsotropicEffectiveFluidThermalConductivity<Derived>::validParams()
{
  return FunctorEffectiveFluidThermalConductivity::validParams();
}

template <typename Derived>
FunctorIsotropicEffectiveFluidThermalConductivity<
    Derived>::FunctorIsotropicEffectiveFluidThermalConductivity(const InputParameters & parameters)
  : FunctorEffectiveFluidThermalConductivity(parameters)
{
  addFunctorProperty<ADRealVectorValue>(
      NS::kappa,
      [this](const auto & r, const auto & t) -> ADRealVectorValue
      {
        RealVectorValue multipliers(1.0, 1.0, 1.0);
        return multipliers * static_cast<Derived *>(this)->computeEffectiveConductivity(r, t);
      });
}
