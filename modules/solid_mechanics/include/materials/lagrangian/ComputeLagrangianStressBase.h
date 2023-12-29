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
#include "RankTwoTensorForward.h"
#include "RankFourTensorForward.h"

/// Provide stresses in the form required for the Lagrangian kernels
///
/// This base class represents a material interface "contract"
///
/// As input it takes the deformation measures provided by
/// ComputeLagrangianStrain:
///   1) "deformation_gradient" -- the deformation gradient
///   2) "inv_inc_def_grad" -- the inverse incremental deformation gradient
///   3) "inv_def_grad" -- the inverse deformation gradient
///   4) "mechanical_strain" -- the integrated deformation rate (log strain)
///   5) "strain_inc" -- the increment in the deformation rate
///
/// Additional strain measures could be defined by subclassing
/// ComputeLagrangianStrain
///
/// And return:
///   1) "stress" -- the cauchy stress
///   2) "cauchy_jacobian" -- the derivative of the increment in the
///                           cauchy stress with respect to the increment
///                           in the spatial velocity gradient
///   3) "pk1_stress" -- the first Piola-Kirchhoff stress
///   4) "pk1_jacobian" -- the derivative of the first PK stress with respect
///                           to the deformation gradient
///
/// Both stress measures and derivatives are required to interface
/// with both the total and updated Lagrangian kernels.
///
/// The class takes the "large_kinematics" flag as input
///
/// If true then the above stress and strain measures will be different
/// and the class is required to use correct large deformation kinematics
///
/// If false all the stress measures and all the strain measures except
/// the deformation gradient are equivalent and the stress update can use
/// small deformation kinematics.  The deformation gradient is the identity
/// for small deformation kinematics, as we use it as a push-forward in the
/// kernel.
///
class ComputeLagrangianStressBase : public Material
{
public:
  static InputParameters validParams();
  ComputeLagrangianStressBase(const InputParameters & parameters);

protected:
  /// Initialize everything with zeros
  virtual void initQpStatefulProperties() override;
  /// Update all properties (here just the stress/derivatives)
  virtual void computeQpProperties() override;
  /// Provide for the actual stress updates
  virtual void computeQpStressUpdate() = 0;

protected:
  /// If true use large deformations
  const bool _large_kinematics;

  /// Prepend to the material properties
  const std::string _base_name;

  /// The Cauchy stress
  MaterialProperty<RankTwoTensor> & _cauchy_stress;
  /// The derivative of the Cauchy stress wrt the increment in the spatial
  /// velocity gradient
  MaterialProperty<RankFourTensor> & _cauchy_jacobian;

  /// The 1st Piola-Kirchhoff stress
  MaterialProperty<RankTwoTensor> & _pk1_stress;
  /// The derivative of the 1st PK stress wrt the deformation gradient
  MaterialProperty<RankFourTensor> & _pk1_jacobian;
};
