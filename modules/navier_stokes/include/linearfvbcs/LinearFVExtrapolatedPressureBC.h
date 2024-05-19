//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVAdvectionDiffusionExtrapolatedBC.h"

/**
 * Class implementing an extrapolated boundary condition for linear finite
 * volume pressure variables. For cases where the pressure is pinned (so no dirichlet BCs are
 * applied), we need to extrapolate the pressure when we compute the gradients but we can't add a
 * source term to the boundary (so no lagged values can go to the boundary).
 * This means that for the time being we need to build the pressure diffusion system
 * assuming a one-term expansion even if the user requested two.
 * For other purposes the extrapolation can be controlled using the `use_two_term_expansion`
 * parameter.
 */
class LinearFVExtrapolatedPressureBC : public LinearFVAdvectionDiffusionExtrapolatedBC
{
public:
  static InputParameters validParams();

  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  LinearFVExtrapolatedPressureBC(const InputParameters & parameters);

  /**
   * These will ensure that the linear system will see one-term expansion only.
   */
  ///@{
  virtual Real computeBoundaryGradientMatrixContribution() const override;
  virtual Real computeBoundaryGradientRHSContribution() const override;
  ///@}
};
