/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CompositeMobilityTensor.h"

template <>
InputParameters
validParams<CompositeMobilityTensor>()
{
  InputParameters params = CompositeTensorBase<RealTensorValue, Material>::validParams();
  params.addClassDescription("Assemble a mobility tensor from multiple tensor contributions "
                             "weighted by material properties");
  params.addRequiredParam<MaterialPropertyName>("M_name",
                                                "Name of the mobility tensor property to generate");
  return params;
}

CompositeMobilityTensor::CompositeMobilityTensor(const InputParameters & parameters)
  : CompositeTensorBase<RealTensorValue, Material>(parameters),
    _M_name(getParam<MaterialPropertyName>("M_name")),
    _M(declareProperty<RealTensorValue>(_M_name))
{
  initializeDerivativeProperties(_M_name);
}

void
CompositeMobilityTensor::computeQpProperties()
{
  computeQpTensorProperties(_M);
}
