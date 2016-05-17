/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef COSSERATLINEARELASTICMATERIAL_H
#define COSSERATLINEARELASTICMATERIAL_H

#include "TensorMechanicsMaterial.h"

/**
 * CosseratLinearElasticMaterial calculates stresses and strains
 * in the Cosserat linear-elastic small-strain case.
 */
class CosseratLinearElasticMaterial : public TensorMechanicsMaterial
{
public:
  CosseratLinearElasticMaterial(const InputParameters & parameters);

protected:
  /**
   * Computes:
   * elastic_strain_ij = grad_j disp_i + epsilon_ijk wc_k
   * curvature_ij = grad_j wc_i
   * where disp=displacement and wc=Cosserat rotation
   */
  virtual void computeQpStrain();

  /**
   * Computes:
   * stress = C_ijkl * (elastic_strain + stress_free_strain)
   * stress_couple = B_ijkl * curvature
   */
  virtual void computeQpStress();

  /**
   * Computes the stress-free strain
   * @return applied_strain - thermal_expansion_coeff * (temperature - T0)
   */
  virtual RankTwoTensor computeStressFreeStrain();

  /**
   * Sets the elasticity tensor (C_ijkl) and the
   * elastic flexural rigidity tensor (B_ijkl)
   */
  virtual void computeQpElasticityTensor();

  /// curvature_ij = grad_j wc_i
  MaterialProperty<RankTwoTensor> & _curvature;

  /// stress_couple_ij = B_ijkl * curvature_kl
  MaterialProperty<RankTwoTensor> & _stress_couple;

  /// _B_ijkl
  MaterialProperty<RankFourTensor> & _elastic_flexural_rigidity_tensor;

  /// _B_ijkl - this goes into the Jacobian calculations in Kernels
  MaterialProperty<RankFourTensor> & _Jacobian_mult_couple;

  /// The vector description of the rank-four tensor B_ijkl
  std::vector<Real> _Bijkl_vector;

  /// The elastic flexural rigidity tensor describing the Cosserat constitutive relation
  RankFourTensor _Bijkl;

  /// Temperature
  const VariableValue & _T;

  /// Thermal expansion coefficient
  const Real _thermal_expansion_coeff;

  /// Reference temperature
  const Real _T0;

  /// Vector description of the applied strain vector (only symmetric applied strains are assumed at present)
  std::vector<Real> _applied_strain_vector;

  /// The applied strain tensor (at present it is assumed this is symmetric)
  RankTwoTensor _applied_strain_tensor;

  /// the number of Cosserat rotation variables supplied by the user (must be 3 in current implementation)
  const unsigned int _nrots;

  /// The Cosserat rotations
  std::vector<const VariableValue *> _wc;

  /// Grad(Cosserat rotation)
  std::vector<const VariableGradient *> _grad_wc;

  /// determines the translation from B_ijkl to the Rank-4 tensor
  const MooseEnum _fill_method_bending;
};

#endif //COSSERATLINEARELASTICMATERIAL_H
