//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVFluxKernel.h"

/**
 * Kernel that adds contributions from a diffusion term discretized using the finite volume method
 * to a linear system.
 */
class LinearFVDiffusion : public LinearFVFluxKernel
{
public:
  static InputParameters validParams();

  /**
   * Class constructor.
   * @param params The InputParameters for the kernel.
   */
  LinearFVDiffusion(const InputParameters & params);

  virtual void initialSetup() override;

  virtual Real computeElemMatrixContribution() override;

  virtual Real computeNeighborMatrixContribution() override;

  virtual Real computeElemRightHandSideContribution() override;

  virtual Real computeNeighborRightHandSideContribution() override;

  virtual Real computeBoundaryMatrixContribution(const LinearFVBoundaryCondition & bc) override;

  virtual Real computeBoundaryRHSContribution(const LinearFVBoundaryCondition & bc) override;

protected:
  /**
   * Computes the matrix contribution from the diffusive face flux.
   * This routine is used to cache the face contribution which
   * would be the same with different signs for the cell and neighbor
   * values.
   */
  Real computeFluxMatrixContribution();

  /**
   * Computes the right hand side contribution from the diffusive face flux.
   * This routine is used to cache the face contribution which
   * would be the same with different signs for the cell and neighbor
   * values.
   */
  Real computeFluxRHSContribution();

  /// The functor for the diffusion coefficient
  const Moose::Functor<Real> & _diffusion_coeff;

  /// Switch to enable/disable nonorthogonal correction
  const bool _use_nonorthogonal_correction;

  /// The cached matrix contribution
  Real _flux_matrix_contribution;

  /// The cached right hand side contribution
  Real _flux_rhs_contribution;
};
