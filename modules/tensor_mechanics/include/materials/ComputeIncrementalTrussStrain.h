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
 * ComputeIncrementalTrussStrain defines a displacement strain increment and rotation
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

  /// Computes the stiffness matrices
  void computeStiffnessMatrix();

  std::vector<MooseVariable *> _disp_var;

  /// Number of coupled displacement variables
  unsigned int _ndisp;

  /// Coupled variable for the truss cross-sectional area
  const VariableValue & _area;

  /// Material stiffness vector that relates displacement strain increments to force increments
  const MaterialProperty<Real> & _material_stiffness;

  /// Stiffness matrix between displacement DOFs of same node or across nodes
  MaterialProperty<Real> & _e_over_l;

  /// Initial length of the truss
  MaterialProperty<Real> & _original_length;

  /// Current length of the truss
  MaterialProperty<Real> & _current_length;

  /// Boolean flag to turn on large strain calculation
  const bool _large_strain;

  /// Rotational transformation from global coordinate system to initial truss local configuration
  RankTwoTensor _original_local_config;

  /// Current total displacement strain integrated over the cross-section in global coordinate system.
  MaterialProperty<RealVectorValue> & _total_disp_strain;

  /// Old total displacement strain integrated over the cross-section in global coordinate system.
  const MaterialProperty<RealVectorValue> & _total_disp_strain_old;

  /// Mechanical displacement strain increment (after removal of eigenstrains) integrated over the cross-section.
  MaterialProperty<RealVectorValue> & _mech_disp_strain_increment;

  /// Vector of truss eigenstrain names
  std::vector<MaterialPropertyName> _eigenstrain_names;

  /// Vector of current displacement eigenstrains
  std::vector<const MaterialProperty<RealVectorValue> *> _disp_eigenstrain;

  /// Vector of old displacement eigenstrains
  std::vector<const MaterialProperty<RealVectorValue> *> _disp_eigenstrain_old;
};
