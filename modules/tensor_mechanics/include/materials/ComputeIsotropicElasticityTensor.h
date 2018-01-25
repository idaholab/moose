//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTEISOTROPICELASTICITYTENSOR_H
#define COMPUTEISOTROPICELASTICITYTENSOR_H

#include "ComputeElasticityTensorBase.h"

class ComputeIsotropicElasticityTensor;

template <>
InputParameters validParams<ComputeIsotropicElasticityTensor>();

/**
 * ComputeIsotropicElasticityTensor defines an elasticity tensor material for
 * isotropic materials.
 */
class ComputeIsotropicElasticityTensor : public ComputeElasticityTensorBase
{
public:
  ComputeIsotropicElasticityTensor(const InputParameters & parameters);

protected:
  virtual void computeQpElasticityTensor() override;

  /// Elastic constants
  bool _bulk_modulus_set;
  bool _lambda_set;
  bool _poissons_ratio_set;
  bool _shear_modulus_set;
  bool _youngs_modulus_set;

  Real _bulk_modulus;
  Real _lambda;
  Real _poissons_ratio;
  Real _shear_modulus;
  Real _youngs_modulus;

  /// Individual elasticity tensor
  RankFourTensor _Cijkl;
};

#endif // COMPUTEISOTROPICELASTICITYTENSOR_H
