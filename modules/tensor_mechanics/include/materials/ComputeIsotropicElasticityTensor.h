/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEISOTROPICELASTICITYTENSOR_H
#define COMPUTEISOTROPICELASTICITYTENSOR_H

#include "ComputeElasticityTensorBase.h"

/**
 * ComputeIsotropicElasticityTensor defines an elasticity tensor material for
 * isotropic materials.
 */
class ComputeIsotropicElasticityTensor : public ComputeElasticityTensorBase
{
public:
  ComputeIsotropicElasticityTensor(const InputParameters & parameters);

protected:
  virtual void computeQpElasticityTensor();

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
