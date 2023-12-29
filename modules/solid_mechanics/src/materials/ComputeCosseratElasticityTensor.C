//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeCosseratElasticityTensor.h"

registerMooseObject("TensorMechanicsApp", ComputeCosseratElasticityTensor);

InputParameters
ComputeCosseratElasticityTensor::validParams()
{
  InputParameters params = ComputeElasticityTensorBase::validParams();
  params.addClassDescription("Compute Cosserat elasticity and flexural bending rigidity tensors");
  params.addRequiredParam<std::vector<Real>>("E_ijkl", "Elastic stiffness tensor for material");
  params.addParam<MooseEnum>(
      "fill_method", RankFourTensor::fillMethodEnum() = "symmetric9", "The fill method");
  params.addRequiredParam<std::vector<Real>>("B_ijkl", "Flexural bending rigidity tensor.");
  params.addParam<MooseEnum>("fill_method_bending",
                             RankFourTensor::fillMethodEnum() = "antisymmetric_isotropic",
                             "The fill method for the 'bending' tensor.");
  return params;
}

ComputeCosseratElasticityTensor::ComputeCosseratElasticityTensor(const InputParameters & parameters)
  : ComputeElasticityTensorBase(parameters),
    _Eijkl(getParam<std::vector<Real>>("E_ijkl"),
           (RankFourTensor::FillMethod)(int)getParam<MooseEnum>("fill_method")),
    _Bijkl(getParam<std::vector<Real>>("B_ijkl"),
           (RankFourTensor::FillMethod)(int)getParam<MooseEnum>("fill_method_bending")),
    _elastic_flexural_rigidity_tensor(
        declareProperty<RankFourTensor>("elastic_flexural_rigidity_tensor"))
{
  if (!isParamValid("elasticity_tensor_prefactor"))
    issueGuarantee(_elasticity_tensor_name, Guarantee::CONSTANT_IN_TIME);
}

void
ComputeCosseratElasticityTensor::computeQpElasticityTensor()
{
  _elasticity_tensor[_qp] = _Eijkl;
  _elastic_flexural_rigidity_tensor[_qp] = _Bijkl;
}
