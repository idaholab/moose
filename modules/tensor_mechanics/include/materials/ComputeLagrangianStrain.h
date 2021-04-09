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
#include "DerivativeMaterialInterface.h"
#include "RankFourTensor.h"
#include "RankTwoTensor.h"

/// Calculate strains to use the MOOSE materials with the Lagrangian kernels
//    This class calculates strain measures used by ComputeLagrangianStress
//    derived materials and used with
//    UpdatedLagrangianStressDivergence and TotalLagrangianStressDivergence
//    kernels
//
//    It has two basic jobs
//    1) Calculate the deformation gradient at time steps n+1 and n
//       (the MOOSE material system doesn't bother for the SmallStrain case)
//    2) Calculate the kinematic quantities needed by the kernels:
//      a) The incremental inverse deformation gradient
//      b) The inverse deformation gradient
//      c) The determinant of the current deformation gradient
//
//    This object cooperates with the homogenization constraint system by
//    adding in the scalar field representing the macroscale displacement
//    gradient before calculating strains.
//
class ComputeLagrangianStrain : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();
  ComputeLagrangianStrain(const InputParameters & parameters);
  virtual ~ComputeLagrangianStrain(){};

protected:
  virtual void initialSetup() override;
  virtual void initQpStatefulProperties() override;
  /// Update all the kinematic quantities
  virtual void computeQpProperties() override;

private:
  /// Calculate the strains based on the spatial velocity gradient
  void _calculateIncrementalStrains(RankTwoTensor L);
  /// Calculate the eigenstrain increment to subtract from the total strain
  /// increment
  RankTwoTensor _eigenstrainIncrement();
  /// Calculate the homogenization macrogradient based on the scalar variable
  /// values
  RankTwoTensor _homogenizationContribution();

protected:
  // Displacements and displacement gradients
  unsigned int _ndisp;
  std::vector<const VariableValue *> _disp;
  std::vector<const VariableGradient *> _grad_disp;
  std::vector<const VariableValue *> _disp_old;
  std::vector<const VariableGradient *> _grad_disp_old;

  /// If true the equilibrium conditions is calculated with large deformations
  bool _ld;

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

  /// Inverse incremental deformation gradient
  MaterialProperty<RankTwoTensor> & _inv_df;
  /// Inverse deformation gradient
  MaterialProperty<RankTwoTensor> & _inv_def_grad;
  /// Volume change
  MaterialProperty<Real> & _detJ;

  /// The scalar variables providing the homogenization strain
  const VariableValue & _macro_gradient;
  /// The "reshaped" actual homogenization gradient contribution
  //    This is optional, could be removed without affecting the formulation
  //    It's just nice to have for output
  MaterialProperty<RankTwoTensor> & _homogenization_contribution;
};
