//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"
#include "InterfaceKernel.h"
#include "ADInterfaceKernel.h"
#include "SBMSurfaceMeshBuilder.h"
#include "BoundaryShortestDistanceToSurface.h"

#include <memory>

/**
 * Template base providing signed-distance and normal-vector utilities for
 * SBM objects. This mixes in with different MOOSE parents (IntegratedBC,
 * InterfaceKernel, ... ) while sharing a single implementation.
 */
template <typename Parent>
class SBMBase : public Parent
{
public:
  static InputParameters validParams();
  SBMBase(const InputParameters & parameters);

protected:
  virtual void initialSetup() override;

  /**
   * Whether to apply shifted integration corrections.
   *
   * Shifted integration is the intended SBM mode. A derived class may return false to retain the
   * same formulation without its shifted corrections as a verification or comparison baseline.
   */
  virtual bool perform_shifted() const = 0;

  /// Returns the characteristic mesh size, computed as (element_volume)^(1/dim)
  Real h() const;

  /// Returns the shortest distance vector from the current quadrature point to the true interface.
  const RealVectorValue surrogateDistance() const;

  /// Returns the true normal vector at the projected point on the true interface.
  const RealVectorValue trueNormal() const;

  /// user object for distance and normal calculations
  const BoundaryShortestDistanceToSurface * _sbm_distance_uo;
};
