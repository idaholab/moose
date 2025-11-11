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
 * Class implementing the base for the Robin boundary condition for linear finite
 * volume variables.
 * alpha, beta, gamma are defined as functors in derived classes.
 */
class LinearFVAdvectionDiffusionFunctorRobinBCBase : public LinearFVAdvectionDiffusionBC
{
public:
  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  LinearFVAdvectionDiffusionFunctorRobinBCBase(const InputParameters & parameters);

  static InputParameters validParams();

  virtual Real computeBoundaryValue() const override;

  virtual Real computeBoundaryNormalGradient() const override;

  virtual Real computeBoundaryValueMatrixContribution() const override;

  virtual Real computeBoundaryValueRHSContribution() const override;

  virtual Real computeBoundaryGradientMatrixContribution() const override;

  virtual Real computeBoundaryGradientRHSContribution() const override;

protected:
  /// Getter functions (consistent entry point for all derived classes)
  virtual Real getAlpha(Moose::FaceArg face, Moose::StateArg state) const = 0;
  virtual Real getBeta(Moose::FaceArg face, Moose::StateArg state) const = 0;
  virtual Real getGamma(Moose::FaceArg face, Moose::StateArg state) const = 0;
};
