//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADCOMPUTEHYPERELASTOPLASTICPFFRACTURESTRESS_H
#define ADCOMPUTEHYPERELASTOPLASTICPFFRACTURESTRESS_H

#include "ADComputeStressBase.h"
#include "ADMaterial.h"

#define usingComputeHyperElastoPlasticPFFractureStressMembers                                      \
  usingComputeStressBaseMembers;                                                                   \
  using usingComputeHyperElastoPlasticPFFractureStress<compute_stage>::_elasticity_tensor;         \
  using usingComputeHyperElastoPlasticPFFractureStress<compute_stage>::_elasticity_tensor_name;    \
  using usingComputeHyperElastoPlasticPFFractureStress<compute_stage>::_hist;                      \
  using usingComputeHyperElastoPlasticPFFractureStress<compute_stage>::_be;                        \
  using usingComputeHyperElastoPlasticPFFractureStress<compute_stage>::_be_old;                    \
  using usingComputeHyperElastoPlasticPFFractureStress<compute_stage>::_deformation_gradient_old;  \
  using usingComputeHyperElastoPlasticPFFractureStress<compute_stage>::_Cp

template <ComputeStage>
class ADComputeHyperElastoPlasticPFFractureStress;

declareADValidParams(ADComputeHyperElastoPlasticPFFractureStress);

/**
 * ADComputeHyperElastoPlasticPFFractureStress computes the stress following linear elasticity
 * theory (finite strains)
 */
template <ComputeStage compute_stage>
class ADComputeHyperElastoPlasticPFFractureStress : public ADComputeStressBase<compute_stage>
{
public:
  ADComputeHyperElastoPlasticPFFractureStress(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;

  virtual void computeQpStress() override;

  /// Name of the elasticity tensor material property
  const std::string _elasticity_tensor_name;
  /// Elasticity tensor material property
  const ADMaterialProperty(RankFourTensor) & _elasticity_tensor;

  /// Coupled order parameter defining the crack
  const ADVariableValue & _c;

  const VariableValue & _c_old;

  /// Small number to avoid non-positive definiteness at or near complete damage
  const Real _kdamage;

  /// Use current value of history variable
  bool _use_current_hist;

  /// Elastic energy and derivatives, declared in this material

  /// History variable that prevents crack healing, declared in this material
  ADMaterialProperty(Real) & _hist;

  /// Old value of history variable
  const MaterialProperty<Real> & _hist_old;

  ADMaterialProperty(RankTwoTensor) & _be_bar;
  ADMaterialProperty(Real) & _alpha;

  const MaterialProperty<RankTwoTensor> & _deformation_gradient_old;
  const MaterialProperty<RankTwoTensor> & _be_bar_old;
  const MaterialProperty<Real> & _alpha_old;

  const Real _yield_stress;
  const Real _k;

  ADMaterialProperty(RankTwoTensor) & _be;
  ADMaterialProperty(RankTwoTensor) & _Cp;

  usingComputeStressBaseMembers;
};

#endif
