//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorDragCoefficients.h"
#include "NS.h"

/**
 * Abstract base class to compute isotropic drag coefficients, where \f$C_L\f$ and
 * \f$C_Q\f$ are independent of direction. Each of \f$C_L\f$ and\f$C_Q\f$
 * are separated into the calculation of a prefactor and a coefficient,where the
 * total drag coefficient is the multiplication of these two. This separation
 * reduces some code duplication.
 */
template <typename Derived>
class FunctorIsotropicDragCoefficients : public FunctorDragCoefficients
{
public:
  FunctorIsotropicDragCoefficients(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  /// a multiplier for adjusting Darcy coefficients
  Real _darcy_mult;

  /// a multiplier for adjusting Forchheimer coefficients
  Real _forchheimer_mult;
};

template <typename Derived>
InputParameters
FunctorIsotropicDragCoefficients<Derived>::validParams()
{
  auto params = FunctorDragCoefficients::validParams();
  params.addParam<Real>("Darcy_multiplier", 1, "A multiplier to adjust Darcy coefficients");
  params.addParam<Real>(
      "Forchheimer_multiplier", 1, "A multiplier to adjust Forchheimer coefficients");
  return params;
}

template <typename Derived>
FunctorIsotropicDragCoefficients<Derived>::FunctorIsotropicDragCoefficients(
    const InputParameters & parameters)
  : FunctorDragCoefficients(parameters),
    _darcy_mult(getParam<Real>("Darcy_multiplier")),
    _forchheimer_mult(getParam<Real>("Forchheimer_multiplier"))
{
  addFunctorProperty<ADRealVectorValue>(
      NS::cL,
      [this](const auto & r, const auto & t) -> ADRealVectorValue
      {
        RealVectorValue multipliers(1.0, 1.0, 1.0);
        return multipliers * static_cast<Derived *>(this)->computeDarcyPrefactor(r, t) *
               static_cast<Derived *>(this)->computeDarcyCoefficient(r, t) * _darcy_mult;
      });

  addFunctorProperty<ADRealVectorValue>(
      NS::cQ,
      [this](const auto & r, const auto & t) -> ADRealVectorValue
      {
        RealVectorValue multipliers(1.0, 1.0, 1.0);
        return multipliers * static_cast<Derived *>(this)->computeForchheimerPrefactor(r, t) *
               static_cast<Derived *>(this)->computeForchheimerCoefficient(r, t) *
               _forchheimer_mult;
      });
}
