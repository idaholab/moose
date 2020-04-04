//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CompositeMobilityTensor.h"

registerMooseObject("PhaseFieldApp", CompositeMobilityTensor);

InputParameters
CompositeMobilityTensor::validParams()
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
