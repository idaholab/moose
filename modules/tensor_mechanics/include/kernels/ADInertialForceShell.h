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

// Forward Declarations
class ADInertialForceShell;

namespace libMesh
{
class QGauss;
}

class ADInertialForceShell : public ADTimeKernel
{
public:
  static InputParameters validParams();

  ADInertialForceShell(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override { return 0.0; };

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeADOffDiagJacobian() override;
  virtual void computeShellInertialForces(const MooseArray<ADReal> & _ad_coord,
                                          const MooseArray<ADReal> & _ad_JxW);
  virtual void computeResidualForJacobian(ADDenseVector & residual);

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
   * Rotational transformation from global to initial beam local
   * coordinate system
   **/
  ADRankTwoTensor _original_local_config;

  /// Direction along which residual is calculated
  const unsigned int _component;

  /**
   * Current translational and rotational velocities at the two nodes
   * of the beam in the global coordinate system
   **/
  ADRealVectorValue _vel_0, _vel_1, _rot_vel_0, _rot_vel_1;
  ADRealVectorValue _vel_2, _vel_3, _rot_vel_2, _rot_vel_3;
  /**
   * Current translational and rotational velocities at the two nodes
   * of the beam in the global coordinate system
   **/
  ADRealVectorValue _old_vel_0, _old_vel_1, _old_rot_vel_0, _old_rot_vel_1;
  ADRealVectorValue _old_vel_2, _old_vel_3, _old_rot_vel_2, _old_rot_vel_3;
  /**
   * Current translational and rotational accelerations at the two nodes
   * of the beam in the global coordinate system
   **/
  ADRealVectorValue _accel_0, _accel_1, _rot_accel_0, _rot_accel_1;
  ADRealVectorValue _accel_2, _accel_3, _rot_accel_2, _rot_accel_3;
  /**
   * Current translational and rotational velocities at the two nodes
   * of the beam in the initial beam local coordinate system
   **/
  ADRealVectorValue _local_vel_0, _local_vel_1, _local_rot_vel_0, _local_rot_vel_1;
  ADRealVectorValue _local_vel_2, _local_vel_3, _local_rot_vel_2, _local_rot_vel_3;
  /**
   * Current translational and rotational velocities at the two nodes
   * of the beam in the initial beam local coordinate system
   **/
  ADRealVectorValue _local_old_vel_0, _local_old_vel_1, _local_old_rot_vel_0, _local_old_rot_vel_1;
  ADRealVectorValue _local_old_vel_2, _local_old_vel_3, _local_old_rot_vel_2, _local_old_rot_vel_3;
  /**
   * Current translational and rotational accelerations at the two nodes
   * of the beam in the initial beam local coordinate system
   **/
  ADRealVectorValue _local_accel_0, _local_accel_1, _local_rot_accel_0, _local_rot_accel_1;
  ADRealVectorValue _local_accel_2, _local_accel_3, _local_rot_accel_2, _local_rot_accel_3;
  /**
   * Forces and moments at the two end nodes of the beam in the initial
   * beam local configuration
   **/
  std::vector<ADRealVectorValue> _local_force, _local_moment;

  /**
   * Forces and moments at the two end nodes of the beam in the global
   * coordinate system
   **/
  ADRealVectorValue _global_force_0, _global_force_1, _global_moment_0, _global_moment_1;
  ADRealVectorValue _global_force_2, _global_force_3, _global_moment_2, _global_moment_3;

  // AMR
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

  /// Helper vectors
  ADRealVectorValue _x2;
  ADRealVectorValue _x3;

  /// Normal to the element at the 4 nodes.
  std::vector<ADRealVectorValue> _node_normal;

  /// Node 1 g vector in reference configuration (mass matrix)
  std::vector<ADRealVectorValue> _0g1_vector;

  /// Node 2 g vector in reference configuration (mass matrix)
  std::vector<ADRealVectorValue> _0g2_vector;

  /// Node 3 g vector in reference configuration (mass matrix)
  std::vector<ADRealVectorValue> _0g3_vector;

  /// Node 4 g vector in reference configuration (mass matrix)
  std::vector<ADRealVectorValue> _0g4_vector;

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
