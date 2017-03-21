/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeCosseratElasticityTensor.h"

template <>
InputParameters
validParams<ComputeCosseratElasticityTensor>()
{
  InputParameters params = validParams<ComputeElasticityTensorBase>();
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
}

void
ComputeCosseratElasticityTensor::computeQpElasticityTensor()
{
  _elasticity_tensor[_qp] = _Eijkl;
  _elastic_flexural_rigidity_tensor[_qp] = _Bijkl;
}
