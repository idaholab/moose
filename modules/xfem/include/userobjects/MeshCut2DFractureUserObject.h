//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshCut2DUserObjectBase.h"

/**
 * MeshCut2DFractureUserObject:
 * (1) reads in a mesh describing the crack surface
 * (2) uses the mesh to do initial cutting of 2D elements, and
 * (3) grows the mesh by a fixed growth rate when a fracture-integral-based growth criterion is met.
 */

class MeshCut2DFractureUserObject : public MeshCut2DUserObjectBase
{
public:
  static InputParameters validParams();

  MeshCut2DFractureUserObject(const InputParameters & parameters);

  virtual void initialize() override;

protected:
  virtual void findActiveBoundaryGrowth() override;

private:
  /// amount to grow crack by for each xfem update step
  const Real & _growth_increment;

  /// are fracture integrals used for growing crack
  const bool _use_k;
  /// is stress used to grow crack
  const bool _use_stress;

  /// critical k value for crack growth
  const Real _k_critical;
  /// Maximum stress criterion threshold for crack growth.
  const Real _stress_threshold;

  /// Pointer fracture integral ki if available
  const std::vector<Real> * const _ki_vpp;
  /// Pointer fracture integral kii if available
  const std::vector<Real> * const _kii_vpp;
  /// Pointer to crack front stress if available
  const std::vector<Real> * const _stress_vpp;
  /// Pointer to crack front critical k if available
  const std::vector<Real> * const _k_critical_vpp;
};
