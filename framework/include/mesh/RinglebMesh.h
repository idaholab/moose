//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef RINGLEBMESH_H
#define RINGLEBMESH_H

#include "MooseMesh.h"

class RinglebMesh;

template <>
InputParameters validParams<RinglebMesh>();

/**
 * Mesh generated from parameters
 */
class RinglebMesh : public MooseMesh
{
public:
  RinglebMesh(const InputParameters & parameters);
  RinglebMesh(const RinglebMesh & /* other_mesh */) = default;

  // No copy
  RinglebMesh & operator=(const RinglebMesh & other_mesh) = delete;

  virtual std::unique_ptr<MooseMesh> safeClone() const override;

  virtual void buildMesh() override;

protected:
  /// Gamma
  const Real & _gamma;

  /// k is a streamline parameter, i.e. k=constant on each streamline.
  /// kmax corresponds to the "inner" wall. Choosing a larger
  /// kmax leads to a more "bullet"-shaped inner wall, a smaller kmax corresponds to a more "parabolic"
  /// inner wall. Another possible kmax value is 1.5.
  const Real & _kmax;

  /// kmin corresponds to the outer wall
  const Real & _kmin;

  /// How many points to discretize the range q = (0.5, k) into.
  const int & _num_q_pts;

  /// how many "extra" points should be inserted in the final element *in addition to* the equispaced q points.
  const int & _n_extra_q_pts;

  /// how many points in the range k=(kmin, kmax).
  const int & _num_k_pts;

  /// The boundary ids to use for the ringleb mesh.
  const boundary_id_type _inflow_bid, _outflow_bid, _inner_wall_bid, _outer_wall_bid;

  /// This parameter, if true, allows to split the quadrilateral elements into triangular elements.
  const bool & _triangles;
};

#endif /* RINGLEBMESH_H */
