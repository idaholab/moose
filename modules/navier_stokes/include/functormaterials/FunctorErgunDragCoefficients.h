//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorPebbleBedDragCoefficients.h"

/**
 *  Material providing the interphase drag coefficient according to the correlation
 *  provided by \cite ergun.
 *  NOTE: Do not inherit this class, inherit FunctorPebbleBedDragCoefficients or turn this
 *       into a template before inheriting, for the sake of the CRTP static polymorphism
 */
class FunctorErgunDragCoefficients final
  : public FunctorPebbleBedDragCoefficients<FunctorErgunDragCoefficients>
{
  friend class FunctorIsotropicDragCoefficients<FunctorErgunDragCoefficients>;

public:
  FunctorErgunDragCoefficients(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  template <typename Space, typename Time>
  ADReal computeDarcyCoefficient(const Space & r, const Time & t) const;

  template <typename Space, typename Time>
  ADReal computeForchheimerCoefficient(const Space & r, const Time & t) const;
};

template <typename Space, typename Time>
ADReal
FunctorErgunDragCoefficients::computeDarcyCoefficient(const Space &, const Time &) const
{
  return 150.0;
}

template <typename Space, typename Time>
ADReal
FunctorErgunDragCoefficients::computeForchheimerCoefficient(const Space &, const Time &) const
{
  return 1.75;
}
