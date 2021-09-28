//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeVariableIsotropicElasticityTensor.h"
#include "Function.h"

registerMooseObject("TensorMechanicsApp", ADComputeVariableIsotropicElasticityTensor);

InputParameters
ADComputeVariableIsotropicElasticityTensor::validParams()
{
  InputParameters params = ADComputeElasticityTensorBase::validParams();
  params.addClassDescription("Compute an isotropic elasticity tensor for elastic constants that "
                             "change as a function of material properties");
  params.addRequiredParam<MaterialPropertyName>(
      "youngs_modulus", "Name of material property defining the Young's Modulus");
  params.addRequiredParam<MaterialPropertyName>(
      "poissons_ratio", "Name of material property defining the Poisson's Ratio");
  return params;
}

ADComputeVariableIsotropicElasticityTensor::ADComputeVariableIsotropicElasticityTensor(
    const InputParameters & parameters)
  : ADComputeElasticityTensorBase(parameters),
    _youngs_modulus(getADMaterialProperty<Real>("youngs_modulus")),
    _poissons_ratio(getADMaterialProperty<Real>("poissons_ratio"))
{
  // all tensors created by this class are always isotropic
  issueGuarantee(_elasticity_tensor_name, Guarantee::ISOTROPIC);
}

void
ADComputeVariableIsotropicElasticityTensor::computeQpElasticityTensor()
{
  _elasticity_tensor[_qp].fillSymmetricIsotropicEandNu(_youngs_modulus[_qp], _poissons_ratio[_qp]);
}
