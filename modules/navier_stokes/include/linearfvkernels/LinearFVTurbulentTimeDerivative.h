//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVTimeDerivative.h"

/**
 * Kernel that adds contributions from a time derivative term of the turbulent variables,
 * limited in the near-wall regions,
 * discretized using the finite volume method to a linear system.
 */
class LinearFVTurbulentTimeDerivative : public LinearFVTimeDerivative
{
public:
  static InputParameters validParams();

  /**
   * Class constructor.
   * @param params The InputParameters for the kernel.
   */
  LinearFVTurbulentTimeDerivative(const InputParameters & params);

  virtual void initialSetup() override;

  virtual void addMatrixContribution() override;

  virtual void addRightHandSideContribution() override;

protected:
  /// Whether the current element is wall bounded
  bool currentElemIsWallBounded() const;

  /// Wall boundaries
  const std::vector<BoundaryName> & _wall_boundary_names;

  /// List for wall bounded elements
  std::unordered_set<const Elem *> _wall_bounded;
};
