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
 * Class implementing a Neumann boundary condition for linear finite
 * volume variables. This is only applicable for advection-diffusion problems.
 */
class LinearFVAdvectionDiffusionFunctorNeumannBC : public LinearFVAdvectionDiffusionBC
{
public:
  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  LinearFVAdvectionDiffusionFunctorNeumannBC(const InputParameters & parameters);

  static InputParameters validParams();

  virtual Real computeBoundaryValue() const override;

  virtual Real computeBoundaryNormalGradient() const override;

  virtual Real computeBoundaryValueMatrixContribution() const override;

  virtual Real computeBoundaryValueRHSContribution() const override;

  virtual Real computeBoundaryGradientMatrixContribution() const override;

  virtual Real computeBoundaryGradientRHSContribution() const override;

  virtual bool includesMaterialPropertyMultiplier() const override { return true; }

protected:
  /// The functor for this BC (can be variable, function, etc)
  const Moose::Functor<Real> & _functor;

  /// The functor for the diffusion coefficient
  const Moose::Functor<Real> & _diffusion_coeff;
};
