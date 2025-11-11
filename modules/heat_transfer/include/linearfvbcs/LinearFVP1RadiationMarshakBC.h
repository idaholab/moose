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
 * Class implementing a Marshak boundary condition for P1 radiation model in
 * linear finite volume variables. This is only applicable for advection-diffusion problems.
 */
class LinearFVP1RadiationMarshakBC : public LinearFVAdvectionDiffusionBC
{
public:
  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  LinearFVP1RadiationMarshakBC(const InputParameters & parameters);

  static InputParameters validParams();

  virtual Real computeBoundaryValue() const override;

  virtual Real computeBoundaryNormalGradient() const override;

  virtual Real computeBoundaryValueMatrixContribution() const override;

  virtual Real computeBoundaryValueRHSContribution() const override;

  virtual Real computeBoundaryGradientMatrixContribution() const override;

  virtual Real computeBoundaryGradientRHSContribution() const override;

protected:
  /// Functor giving the wall temperature
  const Moose::Functor<Real> & _temperature_radiation;
  /// Functor giving the P1 model diffusion coefficient
  const Moose::Functor<Real> & _coeff_diffusion;
  /// Functor giving the wall emmisivity
  const Real & _eps_boundary;
};
