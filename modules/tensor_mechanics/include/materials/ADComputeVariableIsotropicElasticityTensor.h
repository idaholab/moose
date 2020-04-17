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
 * ADComputeVariableIsotropicElasticityTensor defines an elasticity tensor material for
 * isotropic materials in which the elastic constants (Young's modulus and Poisson's ratio)
 * vary as defined by material properties.
 */
class ADComputeVariableIsotropicElasticityTensor : public ADComputeElasticityTensorBase
{
public:
  static InputParameters validParams();

  ADComputeVariableIsotropicElasticityTensor(const InputParameters & parameters);

protected:
  virtual void computeQpElasticityTensor() override;

  /// Material defining the Young's Modulus
  const ADMaterialProperty<Real> & _youngs_modulus;

  /// Material defining the Poisson's Ratio
  const ADMaterialProperty<Real> & _poissons_ratio;

  using ADComputeElasticityTensorBase::_elasticity_tensor;
  using ADComputeElasticityTensorBase::_qp;
  using ADComputeElasticityTensorBase::issueGuarantee;
};
