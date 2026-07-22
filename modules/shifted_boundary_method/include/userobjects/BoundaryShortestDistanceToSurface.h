//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideUserObject.h"

#include <vector>

class Function;

class BoundaryShortestDistanceToSurface : public SideUserObject
{
public:
  using ElemSide = std::pair<dof_id_type, unsigned short int>;

  static InputParameters validParams();
  BoundaryShortestDistanceToSurface(const InputParameters & parameters);

  virtual void execute() override;
  virtual void finalize() override;
  virtual void initialize() override {}
  virtual void threadJoin(const UserObject & /*uo*/) override {}

  /// Provides query interfaces for distance vector
  const RealVectorValue & surrogateDistance(const ElemSide & elem_side, unsigned int qp) const;
  /// Provides query interfaces for normal vector
  const RealVectorValue & trueNormal(const ElemSide & elem_side, unsigned int qp) const;

protected:
  /// Local distance functions when no external distance user object is supplied.
  std::vector<const Function *> _distance_functions;

  /// distance
  std::map<ElemSide, std::vector<RealVectorValue>> _distance_vectors;

  /// normal
  mutable std::map<ElemSide, std::vector<RealVectorValue>> _normal_vectors;

  /// Whether to flip the normal vectors
  std::vector<bool> _flip_normals;

  /// side id to index map
  std::map<BoundaryID, unsigned int> _side_id_index;

private:
  /// @brief If true, the local true normal direction will be corrected to match the direction of the local surrogate normal.
  bool _local_true_normal_correct;

  /// @brief Whether to correct the true normal by integral results.
  bool _correct_true_normal_by_integral;

  /// @brief Sum of dot products between surrogate and true normals for each boundary.
  std::vector<Real> _surrogate_dot_true_normal_sums;

  /// @brief Whether the sums has been reduced across processors.
  bool _surrogate_dot_true_normal_sums_has_reduced;

  /// @brief Map from ElemSide to boundary ID index.
  mutable std::map<ElemSide, unsigned int> _elem_side_to_bid;

  /// @brief Whether to neglect warnings about large distances.
  bool _neglect_distance_warning;

  /// @brief Whether to output debug information.
  bool _debug_output;
};
