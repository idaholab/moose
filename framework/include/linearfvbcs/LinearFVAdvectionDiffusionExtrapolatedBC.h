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
 * Class implementing an extrapolated boundary condition for linear finite
 * volume variables. This is compatible with advection-diffusion problems.
 */
class LinearFVAdvectionDiffusionExtrapolatedBC : public LinearFVAdvectionDiffusionBC
{
public:
  static InputParameters validParams();

  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  LinearFVAdvectionDiffusionExtrapolatedBC(const InputParameters & parameters);

  virtual Real computeBoundaryValue() const override;

  virtual Real computeBoundaryNormalGradient() const override;

  virtual Real computeBoundaryValueMatrixContribution() const override;

  virtual Real computeBoundaryValueRHSContribution() const override;

  virtual Real computeBoundaryGradientMatrixContribution() const override;

  virtual Real computeBoundaryGradientRHSContribution() const override;

protected:
  /// Switch for enabling linear extrapolation for the boundary face value
  const bool _two_term_expansion;
};
