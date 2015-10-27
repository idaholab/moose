/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CompositeMobilityTensor.h"

template<>
InputParameters validParams<CompositeMobilityTensor>()
{
  InputParameters params = CompositeTensorBase<RealTensorValue>::validParams();
  params.addClassDescription("Assemble a mobility tensor from multiple tensor contributions weighted by material properties");
  params.addRequiredParam<MaterialPropertyName>("M_name", "Name of the mobility tensor property to generate");
  return params;
}

CompositeMobilityTensor::CompositeMobilityTensor(const InputParameters & parameters) :
    CompositeTensorBase<RealTensorValue>(parameters)
{
  _M_name = getParam<MaterialPropertyName>("M_name");
  initializeProperties();
}
