//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADStressDivergenceTest.h"

registerADMooseObject("TensorMechanicsTestApp", ADStressDivergenceTest);

defineADValidParams(ADStressDivergenceTest,
                    ADKernel,
                    params.addRequiredParam<unsigned int>("component", "displacement component"););

template <ComputeStage compute_stage>
ADStressDivergenceTest<compute_stage>::ADStressDivergenceTest(const InputParameters & parameters)
  : ADKernel<compute_stage>(parameters),
    _stress(adGetADMaterialProperty<RankTwoTensor>("stress")),
    _component(adGetParam<unsigned int>("component"))
{
}

template <ComputeStage compute_stage>
ADResidual
ADStressDivergenceTest<compute_stage>::computeQpResidual()
{
  return _stress[_qp].row(_component) * _grad_test[_i][_qp];
}
