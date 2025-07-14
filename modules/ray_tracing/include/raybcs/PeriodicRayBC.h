//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralRayBC.h"

class PeriodicBoundaries;

/**
 * RayBC that enforces periodic boundaries.
 */
class PeriodicRayBC : public GeneralRayBC
{
public:
  PeriodicRayBC(const InputParameters & params);

  static InputParameters validParams();

  virtual void onBoundary(const unsigned int num_applying) override final;

  /// Whether or not the RayBC params belong to a PeriodicRayBC
  static bool isPeriodicRayBC(const InputParameters & params);

  /// Name of the parameter that stores the PeriodicBoundaries pointer
  static const std::string periodic_boundaries_param;

private:
  /// The PeriodicBoundaries object
  const libMesh::PeriodicBoundaries & _periodic_boundaries;
  /// Point locator used for searching periodic boundary points
  const std::unique_ptr<libMesh::PointLocatorBase> _point_locator;

  /**
   * State variables for applying periodic boundary conditions.
   *
   * Required in order to enable the application of multiple
   * periodic boundary conditions (via multiple calls to onBoundary())
   * for a single Ray at the same point.
   */
  ///@{
  unsigned int _periodic_applied;
  Point _periodic_point;
  const Elem * _periodic_neighbor;
  ///@}
};
