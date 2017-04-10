/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEVARIABLEISOTROPICELASTICITYTENSOR_H
#define COMPUTEVARIABLEISOTROPICELASTICITYTENSOR_H

#include "ComputeElasticityTensorBase.h"

/**
 * ComputeVariableIsotropicElasticityTensor defines an elasticity tensor material for
 * isotropic materials in which the elastic constants (Young's modulus and Poisson's ratio)
 * vary as defined by material properties.
 */
class ComputeVariableIsotropicElasticityTensor : public ComputeElasticityTensorBase
{
public:
  ComputeVariableIsotropicElasticityTensor(const InputParameters & parameters);

protected:
  virtual void initialSetup();
  virtual void initQpStatefulProperties();
  virtual void computeQpElasticityTensor();

  /// Store the old elasticity tensor to compute the stress correctly for incremental formulations
  MaterialProperty<RankFourTensor> & _elasticity_tensor_old;

  /// Material defininig the Young's Modulus
  const MaterialProperty<Real> & _youngs_modulus;

  /// Material defininig the Poisson's Ratio
  const MaterialProperty<Real> & _poissons_ratio;

  /// number of variables the moduli depend on
  const unsigned int _num_args;

  /// first derivatives of the Young's Modulus with respect to the args
  std::vector<const MaterialProperty<Real> *> _dyoungs_modulus;
  /// second derivatives of the Young's Modulus with respect to the args
  std::vector<std::vector<const MaterialProperty<Real> *>> _d2youngs_modulus;

  /// first derivatives of the Poisson's Ratio with respect to the args
  std::vector<const MaterialProperty<Real> *> _dpoissons_ratio;
  /// second derivatives of the Poisson's Ratio with respect to the args
  std::vector<std::vector<const MaterialProperty<Real> *>> _d2poissons_ratio;

  /// first derivatives of the elasticity tensor with respect to the args
  std::vector<MaterialProperty<RankFourTensor> *> _delasticity_tensor;
  /// second derivatives of the elasticity tensor with respect to the args
  std::vector<std::vector<MaterialProperty<RankFourTensor> *>> _d2elasticity_tensor;

  /// Vector of elastic constants to create the elasticity tensor (member to avoid memory churn)
  std::vector<Real> _isotropic_elastic_constants;
};

#endif // COMPUTEVARIABLEISOTROPICELASTICITYTENSOR_H
