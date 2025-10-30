//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVAdvectionDiffusionExtrapolatedBC.h"

/**
 * Class implementing an outflow boundary condition for linear finite
 * volume variables. This is compatible with advection-diffusion problems.
 */
class LinearFVAdvectionDiffusionOutflowBC : public LinearFVAdvectionDiffusionExtrapolatedBC
{
public:
  static InputParameters validParams();

  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  LinearFVAdvectionDiffusionOutflowBC(const InputParameters & parameters);

  /**
   * We assume zero normal gradient for outflow boundary conditions so these
   * need to be changed
   */
  ///@{
  virtual Real computeBoundaryNormalGradient() const override;
  virtual Real computeBoundaryGradientMatrixContribution() const override;
  virtual Real computeBoundaryGradientRHSContribution() const override;
  ///@}

  virtual bool useBoundaryGradientExtrapolation() const override { return false; }
  virtual bool includesMaterialPropertyMultiplier() const override { return true; }
};
