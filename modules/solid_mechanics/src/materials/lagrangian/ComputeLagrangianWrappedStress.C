//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLagrangianWrappedStress.h"

registerMooseObject("TensorMechanicsApp", ComputeLagrangianWrappedStress);

InputParameters
ComputeLagrangianWrappedStress::validParams()
{
  InputParameters params = ComputeLagrangianObjectiveStress::validParams();

  params.addParam<MaterialPropertyName>("input_stress",
                                        "stress",
                                        "The name of the engineering stress from the "
                                        "wrapped material");
  params.addParam<MaterialPropertyName>("input_jacobian",
                                        "Jacobian_mult",
                                        "The name of the engineering "
                                        "stress/strain Jacobian from the wrapped material");

  return params;
}

ComputeLagrangianWrappedStress::ComputeLagrangianWrappedStress(const InputParameters & parameters)
  : ComputeLagrangianObjectiveStress(parameters),
    _input_stress(getMaterialPropertyByName<RankTwoTensor>(
        _base_name + getParam<MaterialPropertyName>("input_stress"))),
    _input_jacobian(getMaterialPropertyByName<RankFourTensor>(
        _base_name + getParam<MaterialPropertyName>("input_jacobian")))
{
}

void
ComputeLagrangianWrappedStress::computeQpSmallStress()
{
  // Well this ends up being trivial...
  _small_stress[_qp] = _input_stress[_qp];
  _small_jacobian[_qp] = _input_jacobian[_qp];
}
