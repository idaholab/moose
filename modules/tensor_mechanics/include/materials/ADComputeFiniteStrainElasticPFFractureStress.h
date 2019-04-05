//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADCOMPUTEFINITESTRAINELASTICPFFRACTURESTRESS_H
#define ADCOMPUTEFINITESTRAINELASTICPFFRACTURESTRESS_H

#include "ADComputeStressBase.h"
#include "ADMaterial.h"

#define usingComputeFiniteStrainElasticPFFractureStress                                            \
  usingComputeStressBaseMembers;                                                                   \
  using ADComputeLinearElasticStress<compute_stage>::_elasticity_tensor;                           \
  using ADComputeLinearElasticStress<compute_stage>::_elasticity_tensor_name;

template <ComputeStage>
class ADComputeFiniteStrainElasticPFFractureStress;

declareADValidParams(ADComputeFiniteStrainElasticPFFractureStress);

/**
 * ADComputeFiniteStrainElasticPFFractureStress computes the stress following linear elasticity
 * theory (finite strains)
 */
template <ComputeStage compute_stage>
class ADComputeFiniteStrainElasticPFFractureStress : public ADComputeStressBase<compute_stage>
{
public:
  ADComputeFiniteStrainElasticPFFractureStress(const InputParameters & parameters);

protected:
  virtual void computeQpStress();

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

  usingComputeStressBaseMembers;
};

#endif // ADCOMPUTEFINITESTRAINELASTICPFFRACTURESTRESS_H
