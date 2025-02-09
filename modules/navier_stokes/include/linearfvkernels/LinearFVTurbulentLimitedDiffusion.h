//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVDiffusion.h"

/**
 * Kernel that adds contributions from a diffusion term discretized using the finite volume method
 * to a linear system.
 */
class LinearFVTurbulentLimitedDiffusion : public LinearFVDiffusion
{
public:
  static InputParameters validParams();

  /**
   * Class constructor.
   * @param params The InputParameters for the kernel.
   */
  LinearFVTurbulentLimitedDiffusion(const InputParameters & params);

  virtual void initialSetup() override;

  virtual Real computeElemMatrixContribution() override;

  virtual Real computeNeighborMatrixContribution() override;

  virtual Real computeElemRightHandSideContribution() override;

  virtual Real computeNeighborRightHandSideContribution() override;

  virtual Real computeBoundaryMatrixContribution(const LinearFVBoundaryCondition & bc) override;

  virtual Real computeBoundaryRHSContribution(const LinearFVBoundaryCondition & bc) override;

  virtual void addMatrixContribution() override;

  virtual void addRightHandSideContribution() override;

protected:
  /// The functor for the scaling coefficient for the diffusion term
  const Moose::Functor<Real> & _scaling_coeff;

  /// Wall boundaries
  const std::vector<BoundaryName> & _wall_boundary_names;

  /// Maps for wall bounded elements
  std::map<const Elem *, bool> _wall_bounded;
};
