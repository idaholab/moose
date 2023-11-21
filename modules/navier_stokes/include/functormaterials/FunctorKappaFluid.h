//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorIsotropicEffectiveFluidThermalConductivity.h"

/**
 * Template for the fluid effective thermal conductivity based on a
 * volume-average of the thermal conductivity as $\kappa_f=\epsilon k_f$. In other
 * words, no additional effects of thermal dispersion are considered here.
 */
template <typename Derived>
class FunctorKappaFluidTempl : public FunctorIsotropicEffectiveFluidThermalConductivity<Derived>
{
  friend class FunctorIsotropicEffectiveFluidThermalConductivity<Derived>;

public:
  FunctorKappaFluidTempl(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  template <typename Space, typename Time>
  ADReal computeEffectiveConductivity(const Space & r, const Time & t) const;

  /// Fluid thermal conductivity
  const Moose::Functor<ADReal> & _k;
};

template <typename Derived>
template <typename Space, typename Time>
ADReal
FunctorKappaFluidTempl<Derived>::computeEffectiveConductivity(const Space & r, const Time & t) const
{
  return _k(r, t);
}

/**
 * Instantiation of the template for the fluid effective thermal conductivity based on a
 * volume-average of the thermal conductivity as $\kappa_f=\epsilon k_f$. In other
 * words, no additional effects of thermal dispersion are considered here.
 * NOTE: Do not inherit this class, inherit FunctorKappaFluidTempl for the sake of the CRTP
 *       static polymorphism
 */
class FunctorKappaFluid final : public FunctorKappaFluidTempl<FunctorKappaFluid>
{
public:
  FunctorKappaFluid(const InputParameters & parameters);
};

template <typename Derived>
FunctorKappaFluidTempl<Derived>::FunctorKappaFluidTempl(const InputParameters & parameters)
  : FunctorIsotropicEffectiveFluidThermalConductivity<Derived>(parameters),
    _k(FunctorMaterial::getFunctor<ADReal>(NS::k))
{
}

template <typename Derived>
InputParameters
FunctorKappaFluidTempl<Derived>::validParams()
{
  auto params = FunctorIsotropicEffectiveFluidThermalConductivity<Derived>::validParams();
  params.addClassDescription("Zero-thermal dispersion conductivity");
  return params;
}
