//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVAdvectionDiffusionBC.h"

/**
 * Class implementing a Robin boundary condition for linear finite
 * volume variables. This is only applicable for advection-diffusion problems.
 */
class LinearFVAdvectionDiffusionFunctorRobinBC : public LinearFVAdvectionDiffusionBC
{
public:
  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  LinearFVAdvectionDiffusionFunctorRobinBC(const InputParameters & parameters);

  static InputParameters validParams();

  virtual Real computeBoundaryValue() const override;

  virtual Real computeBoundaryNormalGradient() const override;

  virtual Real computeBoundaryValueMatrixContribution() const override;

  virtual Real computeBoundaryValueRHSContribution() const override;

  virtual Real computeBoundaryGradientMatrixContribution() const override;

  virtual Real computeBoundaryGradientRHSContribution() const override;

protected:
  /// Functor giving the alpha coefficient (multiplying normal gradient)
  const Moose::Functor<Real> & _alpha;
  /// Functor giving the beta coefficient (multiplying value)
  const Moose::Functor<Real> & _beta;
  /// Functor giving the gamma coefficient (on right hand side, treated explicitly)
  const Moose::Functor<Real> & _gamma;
};
