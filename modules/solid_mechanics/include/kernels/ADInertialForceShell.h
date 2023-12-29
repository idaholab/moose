//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADTimeKernel.h"
#include "Material.h"
#include "RankTwoTensor.h"
#include "libmesh/dense_vector.h"

namespace libMesh
{
class QGauss;
}

struct PosRotVectors
{
  std::array<ADRealVectorValue, 4> pos;
  std::array<ADRealVectorValue, 4> rot;
};

class ADInertialForceShell : public ADTimeKernel
{
public:
  static InputParameters validParams();

  ADInertialForceShell(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override { return 0.0; };

  virtual void computeResidual() override;
  virtual void computeResidualsForJacobian() override;
  virtual void computeShellInertialForces(const MooseArray<ADReal> & _ad_coord,
                                          const MooseArray<ADReal> & _ad_JxW);

private:
  /// Number of coupled rotational variables
  unsigned int _nrot;

  /// Number of coupled displacement variables
  unsigned int _ndisp;

  /// Variable numbers corresponding to rotational variables
  std::vector<unsigned int> _rot_num;

  /// Variable numbers corresponding to displacement variables
  std::vector<unsigned int> _disp_num;

  /// Variable numbers corresponding to velocity aux variables
  std::vector<unsigned int> _vel_num;

  /// Variable numbers corresponding to acceleraion aux variables
  std::vector<unsigned int> _accel_num;

  /// Variable numbers corresponding to rotational velocity aux variables
  std::vector<unsigned int> _rot_vel_num;

  /// Variable numbers corresponding to rotational acceleration aux variables
  std::vector<unsigned int> _rot_accel_num;

  /**
   * Rotational transformation from global to initial shell local
   * coordinate system
   */
  ADRankTwoTensor _original_local_config;

  /// Direction along which residual is calculated
  const unsigned int _component;

  /// Current shell nodal velocities in the global frame of reference
  PosRotVectors _vel;

  /// Old shell nodal velocities in the global frame of reference
  PosRotVectors _old_vel;

  /// Current shell nodal accelerations in the global frame of reference
  PosRotVectors _accel;

  /// Current shell nodal velocities in the local frame of reference
  PosRotVectors _local_vel;

  /// Old shell nodal velocities in the local frame of reference
  PosRotVectors _local_old_vel;

  /// Current shell nodal accelerations in the local frame of reference
  PosRotVectors _local_accel;

  /**
   * Forces and moments at the four nodes in the initial
   * local configuration
   */
  std::array<ADRealVectorValue, 4> _local_force, _local_moment;

  /**
   * Forces and moments at the four nodes in the global
   * coordinate system
   */
  std::array<ADRealVectorValue, 4> _global_force, _global_moment;

  /// Derivatives of shape functions w.r.t isoparametric coordinates xi
  std::vector<std::vector<Real>> _dphidxi_map;

  /// Derivatives of shape functions w.r.t isoparametric coordinates eta
  std::vector<std::vector<Real>> _dphideta_map;

  /// Shape function value
  std::vector<std::vector<Real>> _phi_map;

  /// Vector storing pointers to the nodes of the shell element
  std::vector<const Node *> _nodes;

  /// Quadrature points in the in-plane direction in isoparametric coordinate system
  std::vector<Point> _2d_points;

  /// Quadrature weights
  std::vector<ADReal> _2d_weights;

  /// First tangential vectors at nodes
  std::vector<ADRealVectorValue> _v1;

  /// Second tangential vectors at nodes
  std::vector<ADRealVectorValue> _v2;

  /// Helper vector
  ADRealVectorValue _x2;

  /// Helper vector
  ADRealVectorValue _x3;

  /// Normal to the element at the 4 nodes.
  std::vector<ADRealVectorValue> _node_normal;

  /// Node 1 g vectors
  std::array<ADRealVectorValue, 2> _0g1_vectors;

  /// Node 2 g vectors
  std::array<ADRealVectorValue, 2> _0g2_vectors;

  /// Node 3 g vectors
  std::array<ADRealVectorValue, 2> _0g3_vectors;

  /// Node 4 g vectors
  std::array<ADRealVectorValue, 2> _0g4_vectors;

  /// Mass proportional Rayleigh damping parameter
  const MaterialProperty<Real> & _eta;

  /// Rotation matrix material property
  const ADMaterialProperty<RankTwoTensor> & _transformation_matrix;

  /// Rotation matrix material property
  const ADMaterialProperty<Real> & _J_map;

  /// Coupled variable for the shell thickness
  const ADReal _thickness;

  /// Shell material density
  const MaterialProperty<Real> & _density;

  /// HHT time integration parameter
  const Real _alpha;
};
