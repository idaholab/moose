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

#define ADInertialForceShellMembers usingTimeKernelMembers

// Forward Declarations
template <ComputeStage compute_stage>
class ADInertialForceShell;

namespace libMesh
{
class QGauss;
}

declareADValidParams(ADInertialForceShell);

template <ComputeStage compute_stage>
class ADInertialForceShell : public ADTimeKernel<compute_stage>
{
public:
  static InputParameters validParams();

  ADInertialForceShell(const InputParameters & parameters);

protected:

  virtual ADReal computeQpResidual() override;

private:
  /// Booleans for validity of params
  const bool _has_velocities;
  const bool _has_rot_velocities;
  const bool _has_accelerations;
  const bool _has_rot_accelerations;

  /// Density of the beam
  const MaterialProperty<Real> & _density;

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

  /// Mass proportional Rayleigh damping parameter
  const MaterialProperty<Real> & _eta;

  /// HHT time integration parameter
  const Real _alpha;

  /**
   * Rotational transformation from global to initial beam local
   * coordinate system
   **/
  const MaterialProperty<RankTwoTensor> & _original_local_config;

  /// Initial length of beam
  const MaterialProperty<Real> & _original_length;

  /// Direction along which residual is calculated
  const unsigned int _component;

  /**
   * Old translational and rotational velocities at the two nodes
   * of the beam in the global coordinate system
   **/
  RealVectorValue _vel_old_0, _vel_old_1, _rot_vel_old_0, _rot_vel_old_1;
  RealVectorValue _vel_old_2, _vel_old_3, _rot_vel_old_2, _rot_vel_old_3;
  /**
   * Current translational and rotational velocities at the two nodes
   * of the beam in the global coordinate system
   **/
  RealVectorValue _vel_0, _vel_1, _rot_vel_0, _rot_vel_1;
  RealVectorValue _vel_2, _vel_3, _rot_vel_2, _rot_vel_3;
  /**
   * Current translational and rotational accelerations at the two nodes
   * of the beam in the global coordinate system
   **/
  RealVectorValue _accel_0, _accel_1, _rot_accel_0, _rot_accel_1;
  RealVectorValue _accel_2, _accel_3, _rot_accel_2, _rot_accel_3;
  /**
   * Old translational and rotational velocities at the two nodes
   * of the beam in the initial beam local coordinate system
   **/
  RealVectorValue _local_vel_old_0, _local_vel_old_1, _local_rot_vel_old_0, _local_rot_vel_old_1;
  RealVectorValue _local_vel_old_2, _local_vel_old_3, _local_rot_vel_old_2, _local_rot_vel_old_3;
  /**
   * Current translational and rotational velocities at the two nodes
   * of the beam in the initial beam local coordinate system
   **/
  RealVectorValue _local_vel_0, _local_vel_1, _local_rot_vel_0, _local_rot_vel_1;
  RealVectorValue _local_vel_2, _local_vel_3, _local_rot_vel_2, _local_rot_vel_3;
  /**
   * Current translational and rotational accelerations at the two nodes
   * of the beam in the initial beam local coordinate system
   **/
  RealVectorValue _local_accel_0, _local_accel_1, _local_rot_accel_0, _local_rot_accel_1;
  RealVectorValue _local_accel_2, _local_accel_3, _local_rot_accel_2, _local_rot_accel_3;
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

  /// Quadrature rule in the out of plane direction
  std::unique_ptr<QGauss> _t_qrule;

  /// Quadrature points in the out of plane direction in isoparametric coordinate system
  std::vector<Point> _t_points;

  /// Quadrature weights in the out of plane direction in isoparametric coordinate system
  std::vector<Real> _t_weights;

  /// Vector storing pointers to the nodes of the shell element
  std::vector<const Node *> _nodes;

  /// Quadrature points in the in-plane direction in isoparametric coordinate system
  std::vector<Point> _2d_points;

  /// Quadrature weights
  std::vector<Real> _2d_weights;

  /// First tangential vectors at nodes
  std::vector<RealVectorValue> _v1;

  /// Second tangential vectors at nodes
  std::vector<RealVectorValue> _v2;

  /// Helper vectors
  RealVectorValue _x2;
  RealVectorValue _x3;

  /// Material property storing the normal to the element at the 4 nodes. Stored as a material property for convinience.
  std::vector<RealVectorValue> _node_normal;

  /// Node 1 g vector in reference configuration (mass matrix)
  std::vector<RealVectorValue> _0g1_vector;

  /// Node 2 g vector in reference configuration (mass matrix)
  std::vector<RealVectorValue> _0g2_vector;

  /// Node 3 g vector in reference configuration (mass matrix)
  std::vector<RealVectorValue> _0g3_vector;

  /// Node 4 g vector in reference configuration (mass matrix)
  std::vector<RealVectorValue> _0g4_vector;

  /// Coupled variable for the shell thickness
  const VariableValue & _thickness;

  usingTimeKernelMembers;
};
