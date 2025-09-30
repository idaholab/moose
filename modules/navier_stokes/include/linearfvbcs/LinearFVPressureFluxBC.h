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
 * H/A flux. This is only applicable for advection-diffusion problems.
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
  /// The H/A flux functor for this BC (can be variable, function, etc)
  const Moose::Functor<Real> & _HbyA_flux;

  /// The functor for the 1/A tensor serving as a diffusion coefficient
  const Moose::Functor<RealVectorValue> & _Ainv;
};
