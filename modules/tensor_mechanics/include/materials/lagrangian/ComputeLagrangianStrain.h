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
#include "RankFourTensorForward.h"
#include "RankTwoTensorForward.h"

/// Calculate strains to use the MOOSE materials with the Lagrangian kernels
///
/// This class calculates strain measures used by ComputeLagrangianStress
/// derived materials and used with
/// UpdatedLagrangianStressDivergence and TotalLagrangianStressDivergence
/// kernels
///
/// It has two basic jobs
/// 1) Calculate the deformation gradient at time steps n+1 and n
///    (the MOOSE material system doesn't bother for the SmallStrain case)
///    This includes including F_bar stabilization, if requested
/// 2) Calculate the kinematic quantities needed by the kernels:
///   a) The incremental inverse deformation gradient
///   b) The inverse deformation gradient
///   c) The determinant of the current deformation gradient
///
/// If required by the stabilize_strain flag it averages the pressure parts
/// of the deformation gradient.
///
/// This object cooperates with the homogenization constraint system by
/// adding in the scalar field representing the macroscale displacement
/// gradient before calculating strains.
///
class ComputeLagrangianStrain : public Material
{
public:
  static InputParameters validParams();
  ComputeLagrangianStrain(const InputParameters & parameters);

protected:
  virtual void initialSetup() override;
  virtual void initQpStatefulProperties() override;
  /// Loop over qp
  virtual void computeProperties() override;
  /// Update all the kinematic quantities
  virtual void computeQpProperties() override;

protected:
  /// Calculate the strains based on the spatial velocity gradient
  virtual void calculateIncrementalStrains(const RankTwoTensor & L);
  /// Subtract the eigenstrain increment to subtract from the total strain
  virtual void subtractEigenstrainIncrement(RankTwoTensor & strain);
  /// Calculate the unstabilized and stabilized deformation gradients
  virtual void calculateDeformationGradient();

  // Displacements and displacement gradients
  const unsigned int _ndisp;
  std::vector<const VariableValue *> _disp;
  std::vector<const VariableGradient *> _grad_disp;

  /// Material system base name
  const std::string _base_name;

  /// If true the equilibrium conditions is calculated with large deformations
  const bool _large_kinematics;

  /// If true stabilize the strains with F_bar
  const bool _stabilize_strain;

  // The eigenstrains
  std::vector<MaterialPropertyName> _eigenstrain_names;
  std::vector<const MaterialProperty<RankTwoTensor> *> _eigenstrains;
  std::vector<const MaterialProperty<RankTwoTensor> *> _eigenstrains_old;

  // The total strains
  MaterialProperty<RankTwoTensor> & _total_strain;
  const MaterialProperty<RankTwoTensor> & _total_strain_old;
  MaterialProperty<RankTwoTensor> & _mechanical_strain;
  const MaterialProperty<RankTwoTensor> & _mechanical_strain_old;

  /// Strain increment
  MaterialProperty<RankTwoTensor> & _strain_increment;

  /// Deformation gradient
  MaterialProperty<RankTwoTensor> & _def_grad;
  /// Old deformation gradient
  const MaterialProperty<RankTwoTensor> & _def_grad_old;

  /// Unstabilized deformation gradient
  MaterialProperty<RankTwoTensor> & _unstabilized_def_grad;

  /// Average deformation gradient (really element-level)
  MaterialProperty<RankTwoTensor> & _avg_def_grad;

  /// Inverse incremental deformation gradient
  MaterialProperty<RankTwoTensor> & _inv_inv_inc_def_grad;
  /// Inverse deformation gradient
  MaterialProperty<RankTwoTensor> & _inv_def_grad;
  /// Volume change
  MaterialProperty<Real> & _detJ;

  /// Names of any extra homogenization gradients
  std::vector<MaterialPropertyName> _homogenization_gradient_names;

  /// Actual homogenization contributions
  std::vector<const MaterialProperty<RankTwoTensor> *> _homogenization_contributions;

  /// Rotation increment for "old" materials inheriting from ComputeStressBase
  MaterialProperty<RankTwoTensor> & _rotation_increment;
};
