//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CohesiveZoneAction.h"
#include "BoundaryShortestDistanceToSurface.h"

/**
 * Action class for the Shifted Cohesive Zone Method (SCZM).
 * * This class inherits from CohesiveZoneAction but overrides the act() method
 * to inject SCZM-specific kernels and materials, allowing for CZM simulations
 * on non-interface-fitted (e.g., background) meshes.
 */
class ShiftedCohesiveZoneAction : public CohesiveZoneAction
{
public:
  static InputParameters validParams();

  ShiftedCohesiveZoneAction(const InputParameters & params);

  /**
   * Main execution point for the Action. Overridden to intercept tasks
   * related to kernel and material addition.
   */
  virtual void act() override;

protected:
  /**
   * Adds the SCZM-specific interface kernels.
   * Note: This does not 'override' the base method because the base is not virtual.
   * Also forwards optional junction-based consistency scaling parameters when supported
   * by the target kernel.
   */
  void addRequiredADSCZMInterfaceKernels();

  /**
   * Adds the SCZM-specific interface materials for jumps and tractions.
   */
  void addRequiredSCZMInterfaceMaterials();
};
