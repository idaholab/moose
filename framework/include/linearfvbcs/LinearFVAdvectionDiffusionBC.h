//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE
#include "LinearFVBoundaryCondition.h"

/**
 * Base class for boundary conditions that are valid for advection diffusion problems.
 * LinearFVAdvection/Diffusion kernels rely on the implementation of the RHS and matrix
 * contribution routines.
 */
class LinearFVAdvectionDiffusionBC : public LinearFVBoundaryCondition
{
public:
  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  LinearFVAdvectionDiffusionBC(const InputParameters & parameters);

  static InputParameters validParams();

  /**
   * Computes the boundary value's contribution to the linear system matrix. Mostly used
   * in advection kernels.
   */
  virtual Real computeBoundaryValueMatrixContribution() const = 0;

  /**
   * Computes the boundary value's contribution to the linear system right hand side.
   * Mostly used in advection kernels.
   */
  virtual Real computeBoundaryValueRHSContribution() const = 0;

  /**
   * Computes the boundary gradient's contribution to the linear system matrix. Mostly used in
   * diffusion kernels.
   */
  virtual Real computeBoundaryGradientMatrixContribution() const = 0;

  /**
   * Computes the boundary gradient's contribution to the linear system right hand side.
   * Mostly used in diffusion kernels.
   */
  virtual Real computeBoundaryGradientRHSContribution() const = 0;

  /**
   * Check if the contributions to the right hand side and matrix already include the material
   * property multiplier. For dirichlet boundary conditions this is false, but for flux boundary
   * conditions this can be true (like Neumann BC for diffusion problems).
   */
  virtual bool includesMaterialPropertyMultiplier() const { return false; }
};
