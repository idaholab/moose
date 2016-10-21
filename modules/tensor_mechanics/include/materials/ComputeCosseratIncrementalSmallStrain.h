/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTECOSSERATINCREMENTALSMALLSTRAIN_H
#define COMPUTECOSSERATINCREMENTALSMALLSTRAIN_H

#include "ComputeCosseratSmallStrain.h"

/**
 * ComputeCosseratIncrementalSmallStrain defines various incremental versions
 * of the Cossserat strain tensor, assuming small strains.
 */
class ComputeCosseratIncrementalSmallStrain : public ComputeCosseratSmallStrain
{
 public:
  ComputeCosseratIncrementalSmallStrain(const InputParameters & parameters);

 protected:
  virtual void computeQpProperties();

  virtual void initQpStatefulProperties();

  /// The Cosserat rotations
  std::vector<const VariableValue *> _wc_old;

  /// Grad(Cosserat rotation)
  std::vector<const VariableGradient *> _grad_wc_old;

  /// strain_increment/dt
  MaterialProperty<RankTwoTensor> & _strain_rate;

  /**
   * strain_ij = grad_j disp_i + epsilon_ijk * cosseratRotation_k
   * strain_increment = (strain - strain_old) - stress_free_strain_increment
   */
  MaterialProperty<RankTwoTensor> & _strain_increment;

  /// mechanical strain = mechanical_strain_old + strain_increment.  In linear elasticity, this goes into the stress = elas * mechanical_strain constitutive law.
  MaterialProperty<RankTwoTensor> & _mechanical_strain_old;

  /// total_strain includes stress_free_strain contributions.  Ie, even if total_strain is nonzero, stress might be zero.
  MaterialProperty<RankTwoTensor> & _total_strain_old;

  /// Remains = 1 through all time because of small strain.
  MaterialProperty<RankTwoTensor> & _rotation_increment;

  /// deformation_gradient = 1 + strain, where the latter includes the contributions from Cosserat rotations
  MaterialProperty<RankTwoTensor> & _deformation_gradient;

  /// increment of stress_free_strain (eg, stress_free_strain = thermal_expansion_coeff * (T - T0), and the "increment" is stress_free_strain - stress_free_strain_old
  const MaterialProperty<RankTwoTensor> & _stress_free_strain_increment;

  /// the Cosserat curvature strain: curvature_ij = nabla_j CosseratRotation_i
  MaterialProperty<RankTwoTensor> & _curvature_old;

  /// _curvature_increment = (curvature - _curvature_old)
  MaterialProperty<RankTwoTensor> & _curvature_increment;
};

#endif //COMPUTECOSSERATINCREMENTALSMALLSTRAIN_H
