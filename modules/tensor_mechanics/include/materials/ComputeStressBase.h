/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTESTRESSBASE_H
#define COMPUTESTRESSBASE_H

#include "Material.h"
#include "RankTwoTensor.h"
#include "ElasticityTensorR4.h"
#include "RotationTensor.h"
#include "DerivativeMaterialInterface.h"

/**
 * ComputeStressBase is the base class for stress tensors
 */
class ComputeStressBase : public DerivativeMaterialInterface<Material>
{
public:
  ComputeStressBase(const std:: string & name, InputParameters parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();
  virtual void computeQpStress() = 0;

  std::string _base_name;

  const MaterialProperty<RankTwoTensor> & _total_strain;
  MaterialProperty<RankTwoTensor> & _stress;
  MaterialProperty<RankTwoTensor> & _elastic_strain;
  const MaterialProperty<ElasticityTensorR4> & _elasticity_tensor;

  /// Extra stress tensor
  const MaterialProperty<RankTwoTensor> & _extra_stress;

  /// initial stress components
  std::vector<Function *> _initial_stress;

  /// derivative of stress w.r.t. strain (_dstress_dstrain)
  MaterialProperty<ElasticityTensorR4> & _Jacobian_mult;
};

#endif //COMPUTESTRESSBASE_H
