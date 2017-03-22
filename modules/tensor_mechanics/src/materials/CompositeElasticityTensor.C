/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CompositeElasticityTensor.h"

template <>
InputParameters
validParams<CompositeElasticityTensor>()
{
  InputParameters params = CompositeTensorBase<RankFourTensor, Material>::validParams();
  params.addClassDescription("Assemble an elasticity tensor from multiple tensor contributions "
                             "weighted by material properties");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  return params;
}

CompositeElasticityTensor::CompositeElasticityTensor(const InputParameters & parameters)
  : CompositeTensorBase<RankFourTensor, Material>(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _M_name(_base_name + "elasticity_tensor"),
    _M(declareProperty<RankFourTensor>(_M_name))
{
  // we take the tensor names to be the _base names_ of the elasticity tensors
  for (unsigned int i = 0; i < _num_comp; ++i)
    _tensor_names[i] += "_elasticity_tensor";

  initializeDerivativeProperties(_M_name);
}

void
CompositeElasticityTensor::computeQpProperties()
{
  computeQpTensorProperties(_M);
}
