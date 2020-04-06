//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseMesh.h"

/**
 * Mesh generated from parameters
 */
class RinglebMesh : public MooseMesh
{
public:
  static InputParameters validParams();

  RinglebMesh(const InputParameters & parameters);
  RinglebMesh(const RinglebMesh & /* other_mesh */) = default;

  // No copy
  RinglebMesh & operator=(const RinglebMesh & other_mesh) = delete;

  virtual std::unique_ptr<MooseMesh> safeClone() const override;

  virtual void buildMesh() override;

  // This function computes the different parameters a, rho, p and J
  std::vector<Real> arhopj(const Real & gamma, const std::vector<Real> & q, const int & index);

  // This function computes the (x,y) coordinates of the nodes
  // The vector `values` can be got with the `arhopj` function above
  std::vector<Real> computexy(const std::vector<Real> values,
                              const int & i,
                              const int & index,
                              const std::vector<Real> & ks,
                              const std::vector<Real> & q);

protected:
  /// Gamma
  const Real & _gamma;

  /// k is a streamline parameter, i.e. k=constant on each streamline.
  /// kmax corresponds to the "inner" wall. Choosing a larger
  /// kmax leads to a more "bullet"-shaped inner wall, a smaller kmax corresponds to a more "parabolic"
  /// inner wall. A possible kmax value is 1.5.
  const Real & _kmax;

  /// kmin corresponds to the outer wall
  const Real & _kmin;

  /// How many points to discretize the range q = (0.5, k) into.
  const int & _num_q_pts;

  /// how many "extra" points should be inserted in the nearest element from the horizontal *in additi  /// on to* the equispaced q points.
  const int & _n_extra_q_pts;

  /// how many points in the range k=(kmin, kmax).
  const int & _num_k_pts;

  /// The boundary ids to use for the ringleb mesh.
  const boundary_id_type _inflow_bid, _outflow_bid, _inner_wall_bid, _outer_wall_bid;

  /// This parameter, if true, allows to split the quadrilateral elements into triangular elements.
  const bool & _triangles;
};
