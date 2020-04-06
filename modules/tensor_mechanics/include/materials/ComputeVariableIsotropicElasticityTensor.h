//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeElasticityTensorBase.h"

/**
 * ComputeVariableIsotropicElasticityTensor defines an elasticity tensor material for
 * isotropic materials in which the elastic constants (Young's modulus and Poisson's ratio)
 * vary as defined by material properties.
 */
class ComputeVariableIsotropicElasticityTensor : public ComputeElasticityTensorBase
{
public:
  static InputParameters validParams();

  ComputeVariableIsotropicElasticityTensor(const InputParameters & parameters);

protected:
  virtual void initialSetup() override;
  virtual void initQpStatefulProperties() override;
  virtual void computeQpElasticityTensor() override;

  /// Material defining the Young's Modulus
  const MaterialProperty<Real> & _youngs_modulus;

  /// Material defining the Poisson's Ratio
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
