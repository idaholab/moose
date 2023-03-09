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
 * (1) uses the mesh to do initial cutting of 2D elements, and
 * (2) grows the mesh by a fixed growth rate.
 */

class MeshCut2DFractureUserObject : public MeshCut2DUserObjectBase
{
public:
  static InputParameters validParams();

  MeshCut2DFractureUserObject(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void initialize() override;

  virtual const std::vector<Point>
  getCrackFrontPoints(unsigned int num_crack_front_points) const override;
  virtual const std::vector<RealVectorValue>
  getCrackPlaneNormals(unsigned int num_crack_front_points) const override;

protected:
  /**
   * Contains the original crack front node ids in pair.first and the
   * assosciated current crack front node id in pair.second.  This vector is sorted on pair.first
   * which makes the ordering of this vector the same as that used in the CrackFrontDefinition
   */
  std::vector<std::pair<dof_id_type, dof_id_type>> _original_and_current_front_node_ids;

  /**
    Find the original crack front nodes in the cutter mesh and fill pair.first in
    _original_and_current_front_node_ids.
   */
  void findOriginalCrackFrontNodes();

  /**
    Grow the cutter mesh
   */
  void growFront();

  /// Gets the time of the previous call to this user object.
  /// This is used to determine if certain things should be executed or not
  Real _time_of_previous_call_to_UO;

  /// The crack front definition
  CrackFrontDefinition * _crack_front_definition;

private:
  const Real _k_critical_squared;
  const Real _growth_per_timestep;

  /**
    Find growth direction at each active node
   */
  std::vector<Point> findActiveBoundaryDirection(const std::vector<Real> & k1,
                                                 const std::vector<Real> & k2) const;

  /// compute k_squared from fracture integrals
  std::vector<Real> getKSquared(const std::vector<Real> & k1, const std::vector<Real> & k2) const;
};
