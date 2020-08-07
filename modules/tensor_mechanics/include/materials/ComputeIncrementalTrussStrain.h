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

  /// Number of coupled displacement variables
  unsigned int _ndisp;

  /// Variable numbers corresponding to the displacement variables
  std::vector<unsigned int> _disp_num;

    /// Coupled variable for the truss cross-sectional area
  const VariableValue & _area;

  MaterialProperty<Real> & _total_stretch;
  MaterialProperty<Real> & _elastic_stretch;

  /// Rotational transformation from global coordinate system to initial truss local configuration
  RankTwoTensor _original_local_config;

  /// Initial length of the truss
  MaterialProperty<Real> & _original_length;

  /// Current length of the truss
  MaterialProperty<Real> & _current_length;

  /// Rotational transformation from global coordinate system to truss local configuration at time t
  MaterialProperty<RankTwoTensor> & _total_rotation;

  /// Current total displacement strain integrated over the cross-section in global coordinate system.
  MaterialProperty<RealVectorValue> & _total_disp_strain;

  /// Old total displacement strain integrated over the cross-section in global coordinate system.
  const MaterialProperty<RealVectorValue> & _total_disp_strain_old;

  /// Mechanical displacement strain increment (after removal of eigenstrains) integrated over the cross-section.
  MaterialProperty<RealVectorValue> & _mech_disp_strain_increment;

  /// Material stiffness vector that relates displacement strain increments to force increments
  const MaterialProperty<Real> & _material_stiffness;

  /// Stiffness matrix between displacement DOFs of same node or across nodes
  MaterialProperty<Real> & _K11;

  /// Boolean flag to turn on large strain calculation
  const bool _large_strain;

  /// Gradient of displacement calculated in the truss local configuration at time t
  RealVectorValue _grad_disp_0_local_t;

  /// Vector of truss eigenstrain names
  std::vector<MaterialPropertyName> _eigenstrain_names;

  /// Vector of current displacement eigenstrains
  std::vector<const MaterialProperty<RealVectorValue> *> _disp_eigenstrain;

  /// Vector of old displacement eigenstrains
  std::vector<const MaterialProperty<RealVectorValue> *> _disp_eigenstrain_old;

  /// Displacement and rotations at the two nodes of the truss in the global coordinate system
  RealVectorValue _disp0, _disp1;

  /// Reference to the nonlinear system object
  NonlinearSystemBase & _nonlinear_sys;

  /// Indices of solution vector corresponding to displacement DOFs at the node 0
  std::vector<unsigned int> _soln_disp_index_0;

  /// Indices of solution vector corresponding to displacement DOFs at the node 1
  std::vector<unsigned int> _soln_disp_index_1;

  /// Rotational transformation from global coordinate system to initial truss local configuration
  MaterialProperty<RankTwoTensor> & _initial_rotation;

  /// Psuedo stiffness for critical time step computation
  MaterialProperty<Real> & _effective_stiffness;

  /// Prefactor function to multiply the elasticity tensor with
  const Function * const _prefactor_function;
};
