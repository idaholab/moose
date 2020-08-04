//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "RankTwoTensor.h"

// Forward Declarations
class Function;

/**
 * ComputeIncrementalTrussStrain defines a displacement and rotation strain increment and rotation
 * increment (=1), for small strains.
 */
class ComputeIncrementalTrussStrain : public Material
{
public:
  static InputParameters validParams();

  ComputeIncrementalTrussStrain(const InputParameters & parameters);

  virtual void computeProperties() override;

protected:
  virtual void initQpStatefulProperties() override;

  /// Computes the displacement and rotation strain increments
  void computeQpStrain();

  // virtual Real computeStiffness(unsigned int i, unsigned int j);

  /// Computes the stiffness matrices
  void computeStiffnessMatrix();

  /// Computes the rotation matrix at time t. For small rotation scenarios, the rotation matrix at time t is same as the intiial rotation matrix
  virtual void computeRotation();

  std::vector<MooseVariable *> _disp_var;
  /// Base name of the material system
  const std::string _base_name;
  // const VariableValue & _youngs_modulus;
  MaterialProperty<Real> & _total_stretch;
  MaterialProperty<Real> & _elastic_stretch;
  // MaterialProperty<Real> & _axial_stress;
  // MaterialProperty<Real> & _e_over_l;

  // /// Number of coupled rotational variables
  // unsigned int _nrot;

  /// Number of coupled displacement variables
  unsigned int _ndisp;

  // /// Variable numbers corresponding to the rotational variables
  // std::vector<unsigned int> _rot_num;

  /// Variable numbers corresponding to the displacement variables
  std::vector<unsigned int> _disp_num;

    /// Coupled variable for the truss cross-sectional area
  const VariableValue & _area;

  Real _origin_length;
  Real _current_length;


  /// Rotational transformation from global coordinate system to initial truss local configuration
  RankTwoTensor _original_local_config;

  /// Initial length of the truss
  MaterialProperty<Real> & _original_length;

  /// Rotational transformation from global coordinate system to truss local configuration at time t
  MaterialProperty<RankTwoTensor> & _total_rotation;

  /// Current total displacement strain integrated over the cross-section in global coordinate system.
  MaterialProperty<RealVectorValue> & _total_disp_strain;

  // /// Current total rotational strain integrated over the cross-section in global coordinate system.
  // MaterialProperty<RealVectorValue> & _total_rot_strain;

  /// Old total displacement strain integrated over the cross-section in global coordinate system.
  const MaterialProperty<RealVectorValue> & _total_disp_strain_old;

  // /// Old total rotational strain integrated over the cross-section in global coordinate system.
  // const MaterialProperty<RealVectorValue> & _total_rot_strain_old;

  /// Mechanical displacement strain increment (after removal of eigenstrains) integrated over the cross-section.
  MaterialProperty<RealVectorValue> & _mech_disp_strain_increment;

  // /// Mechanical rotation strain increment (after removal of eigenstrains) integrated over the cross-section
  // MaterialProperty<RealVectorValue> & _mech_rot_strain_increment;

  /// Material stiffness vector that relates displacement strain increments to force increments
  const MaterialProperty<RealVectorValue> & _material_stiffness;

  /// Stiffness matrix between displacement DOFs of same node or across nodes
  MaterialProperty<RankTwoTensor> & _K11;

  /// Boolean flag to turn on large strain calculation
  const bool _large_strain;

  /// Gradient of displacement calculated in the truss local configuration at time t
  RealVectorValue _grad_disp_0_local_t;

  /// Gradient of rotation calculated in the truss local configuration at time t
  RealVectorValue _grad_rot_0_local_t;

  /// Average rotation calculated in the truss local configuration at time t
  RealVectorValue _avg_rot_local_t;

  /// Vector of truss eigenstrain names
  std::vector<MaterialPropertyName> _eigenstrain_names;

  /// Vector of current displacement eigenstrains
  std::vector<const MaterialProperty<RealVectorValue> *> _disp_eigenstrain;

  // /// Vector of current rotational eigenstrains
  // std::vector<const MaterialProperty<RealVectorValue> *> _rot_eigenstrain;

  /// Vector of old displacement eigenstrains
  std::vector<const MaterialProperty<RealVectorValue> *> _disp_eigenstrain_old;

  // /// Vector of old rotational eigenstrains
  // std::vector<const MaterialProperty<RealVectorValue> *> _rot_eigenstrain_old;

  /// Displacement and rotations at the two nodes of the truss in the global coordinate system
  RealVectorValue _disp0, _disp1, _rot0, _rot1;

  /// Reference to the nonlinear system object
  NonlinearSystemBase & _nonlinear_sys;

  /// Rotational transformation from global coordinate system to initial truss local configuration
  MaterialProperty<RankTwoTensor> & _initial_rotation;

  /// Psuedo stiffness for critical time step computation
  MaterialProperty<Real> & _effective_stiffness;

  /// Prefactor function to multiply the elasticity tensor with
  const Function * const _prefactor_function;
};
