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
#include "Function.h"
#include "RotationTensor.h"

/**
 * This class defines a Hill tensor material object with a given base name.
 */

template <bool is_ad>
class HillConstantsTempl : public Material
{
public:
  static InputParameters validParams();

  HillConstantsTempl(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  virtual void initQpStatefulProperties() override;

  virtual void rotateHillConstants(const std::vector<Real> & hill_constants_input);

  /// Base name of the material system
  const std::string _base_name;

  /// Flag to determine whether to rotate Hill's tensor with large strain kinematics
  const bool _use_large_rotation;

  /// Rotation up to current step "n" to compute Hill tensor
  GenericMaterialProperty<RankTwoTensor, is_ad> * _rotation_total_hill;
  /// Rotation up to "n - 1" (previous) step to compute Hill tensor
  const MaterialProperty<RankTwoTensor> * _rotation_total_hill_old;

  /// Strain increment material property
  const GenericMaterialProperty<RankTwoTensor, is_ad> * _rotation_increment;

  /// Hill constants for orthotropic inelasticity
  const std::vector<Real> _hill_constants_input;
  DenseMatrix<Real> _hill_tensor;

  /// Material property for storing hill constants (unrotated)
  MaterialProperty<std::vector<Real>> & _hill_constant_material;

  /// Material property for storing transformed Hill tensor
  MaterialProperty<DenseMatrix<Real>> * _hill_tensor_material;

  /// Euler angles for transformation of hill tensor
  RealVectorValue _zxz_angles;

  /// Transformation matrix
  DenseMatrix<Real> _transformation_tensor;

  /// Flag to determine if temperature is supplied by the user
  const bool _has_temp;

  /// Temperature variable value
  const VariableValue & _temperature;

  /// Function names
  const std::vector<FunctionName> _function_names;

  /// Number of function names
  const unsigned int _num_functions;

  /// The functions describing the temperature dependence
  std::vector<const Function *> _functions;

  // Initial rigid body rotation of the structural element
  RotationTensor _rigid_body_rotation_tensor;
};

typedef HillConstantsTempl<false> HillConstants;
typedef HillConstantsTempl<true> ADHillConstants;
