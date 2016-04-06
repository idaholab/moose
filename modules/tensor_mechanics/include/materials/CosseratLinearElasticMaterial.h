/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
// Original class author: A.M. Jokisaari,  O. Heinonen

#ifndef COSSERATLINEARELASTICMATERIAL_H
#define COSSERATLINEARELASTICMATERIAL_H

#include "TensorMechanicsMaterial.h"

/**
 * CosseratLinearElasticMaterial handles a fully anisotropic, single-crystal material's elastic
 * constants.  It takes all 21 independent stiffness tensor inputs, or only 9, depending on the
 * boolean input value given.  This can be extended or simplified to specify HCP, monoclinic,
 * cubic, etc as needed.
 */
class CosseratLinearElasticMaterial : public TensorMechanicsMaterial
{
public:
  CosseratLinearElasticMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpStrain();
  virtual void computeQpStress();
  virtual RankTwoTensor computeStressFreeStrain();
  virtual void computeQpElasticityTensor();

  MaterialProperty<RankTwoTensor> & _eigenstrain;
  MaterialProperty<RankTwoTensor> & _symmetric_strain;
  MaterialProperty<RankTwoTensor> & _antisymmetric_strain;
  MaterialProperty<RankTwoTensor> & _curvature;

  MaterialProperty<RankTwoTensor> & _symmetric_stress;
  MaterialProperty<RankTwoTensor> & _antisymmetric_stress;
  MaterialProperty<RankTwoTensor> & _stress_couple;

  MaterialProperty<RankFourTensor> & _elastic_flexural_rigidity_tensor;
  MaterialProperty<RankFourTensor> & _Jacobian_mult_couple;

  std::vector<Real> _Bijkl_vector;
  RankFourTensor _Bijkl;

private:
  const VariableValue & _T;

  Real _thermal_expansion_coeff;
  const Real _T0;
  std::vector<Real> _applied_strain_vector;
  RankTwoTensor _applied_strain_tensor;

  const VariableValue & _wc_x;
  const VariableValue & _wc_y;
  const VariableValue & _wc_z;

  const VariableGradient & _grad_wc_x;
  const VariableGradient & _grad_wc_y;
  const VariableGradient & _grad_wc_z;

  /// determines the translation from B_ijkl to the Rank-4 tensor
  MooseEnum _fill_method_bending;
};

#endif //COSSERATLINEARELASTICMATERIAL_H
