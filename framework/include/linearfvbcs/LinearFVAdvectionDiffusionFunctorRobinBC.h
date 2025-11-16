//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVAdvectionDiffusionFunctorRobinBCBase.h"

/**
 * Class implementing a Robin boundary condition for linear finite
 * volume variables. This is only applicable for advection-diffusion problems.
 * alpha, beta, gamma are provided as functors and may vary in space/time.
 */
class LinearFVAdvectionDiffusionFunctorRobinBC : public LinearFVAdvectionDiffusionFunctorRobinBCBase
{
public:
  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  LinearFVAdvectionDiffusionFunctorRobinBC(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  /// Getter functions (consistent entry point for all derived classes)
  virtual Real getAlpha(Moose::FaceArg face, Moose::StateArg state) const override;
  virtual Real getBeta(Moose::FaceArg face, Moose::StateArg state) const override;
  virtual Real getGamma(Moose::FaceArg face, Moose::StateArg state) const override;

  /// Functor giving the alpha coefficient (multiplying normal gradient)
  const Moose::Functor<Real> & _alpha;
  /// Functor giving the beta coefficient (multiplying value)
  const Moose::Functor<Real> & _beta;
  /// Functor giving the gamma coefficient (on right hand side, treated explicitly)
  const Moose::Functor<Real> & _gamma;
};
