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

class CrackFrontDefinition;

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
  /**
   * Compute all of the maximum hoop stress fracture integrals for all crack trips from the fracture
   * integral vector post processors
   * @param k1 fracture integrals from KI vector postprocessors
   * @param k2 fracture integrals from KII vector postprocessors
   * @return computed fracture integral squared
   */
  std::vector<Real> getKSquared(const std::vector<Real> & k1, const std::vector<Real> & k2) const;

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

  /// dummy vectors for vpps to point to if not defined(these are not const because reporters can change their size)
  std::vector<Real> _zeros_vec;

  /// References fracture integral ki if input
  const std::vector<Real> & _ki_vpp;
  /// References fracture integral kii if input
  const std::vector<Real> & _kii_vpp;
  /// References crack front stress if input
  const std::vector<Real> & _stress_vpp;
};
