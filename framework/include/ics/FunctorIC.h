//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InitialCondition.h"
#include "NonADFunctorInterface.h"

/**
 * Defines an initial condition using a functor
 */
class FunctorIC : public InitialCondition, public NonADFunctorInterface
{
public:
  static InputParameters validParams();

  FunctorIC(const InputParameters & parameters);

protected:
  /**
   * The value of the variable at a point.
   */
  virtual Real value(const Point & p) override;

  /**
   * The value of the gradient at a point.
   */
  virtual RealGradient gradient(const Point & p) override;

  /// Function to evaluate to form the initial condition
  const Moose::Functor<Real> & _functor;

  /// Scaling factor, to be able to use a functor with multiple ICs
  const Real _scaling;
};
