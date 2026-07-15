//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVAdvectionDiffusionBC.h"

/**
 * Class implementing a flux boundary condition for linear finite
 * volume pressure variables used in the pressure corrector equation which is consistent with the
 * H/A flux and a prescribed boundary velocity. This allows the pressure boundary flux to be
 * consistent with a non-zero boundary mass flux.
 */
class LinearFVPressureFluxBC : public LinearFVAdvectionDiffusionBC
{
public:
  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  LinearFVPressureFluxBC(const InputParameters & parameters);

  static InputParameters validParams();

  virtual Real computeBoundaryValue() const override;

  virtual Real computeBoundaryNormalGradient() const override;

  virtual Real computeBoundaryValueMatrixContribution() const override;

  virtual Real computeBoundaryValueRHSContribution() const override;

  virtual Real computeBoundaryGradientMatrixContribution() const override;

  virtual Real computeBoundaryGradientRHSContribution() const override;

  virtual bool includesMaterialPropertyMultiplier() const override { return true; }

protected:
  /// Compute the required boundary pressure flux contribution
  Real computeRequiredPressureFlux() const;

  /// Compute the scalar A^{-1} coefficient used in the pressure BC
  Real computeBoundaryAinv() const;

  /// The H/A flux functor for this BC (can be variable, function, etc)
  const Moose::Functor<Real> & _HbyA_flux;

  /// The functor for the 1/A tensor serving as a diffusion coefficient
  const Moose::Functor<RealVectorValue> & _Ainv;

  /// Spatial dimension of the mesh
  const unsigned short _dim;

  /// Velocity functors used to prescribe a boundary mass flux
  const Moose::Functor<Real> & _u;
  const Moose::Functor<Real> * const _v;
  const Moose::Functor<Real> * const _w;

  /// Density functor used with the prescribed boundary velocity
  const Moose::Functor<Real> & _rho;
};
