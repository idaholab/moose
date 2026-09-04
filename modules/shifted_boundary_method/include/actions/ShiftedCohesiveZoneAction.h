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
  /// Adds the surface distance functions used by the generated distance user object.
  void addSBMDistanceFunctions();

  /// Adds the generated surface geometry and aggregate boundary distance user objects.
  void addSBMDistanceUserObjects();

  /// Returns the saved surface mesh names, defaulting to the boundary names.
  std::vector<MeshGeneratorName> surfaceMeshNames() const;

  /// Returns the generated builder name for a boundary.
  UserObjectName surfaceMeshBuilderName(const BoundaryName & boundary) const;

  /// Returns the generated distance function name for a boundary.
  FunctionName surfaceDistanceFunctionName(const BoundaryName & boundary) const;

  /// Returns the generated aggregate distance user object name.
  UserObjectName sbmDistanceUserObjectName() const;

  /// Returns the generated single-mesh interface manager name.
  UserObjectName interfaceManagerName() const;

  /// Sets SCZM-specific parameters on each generated interface kernel.
  void customizeCZMInterfaceKernel(InputParameters & params) const override;

  /// Sets SCZM-specific parameters on the generated displacement jump material.
  void customizeCZMDisplacementJump(InputParameters & params) const override;
};
