//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeCosseratStressBase.h"

InputParameters
ComputeCosseratStressBase::validParams()
{
  InputParameters params = ComputeStressBase::validParams();
  params.addClassDescription("Compute stress and couple stress in the Cosserat situation");
  return params;
}

ComputeCosseratStressBase::ComputeCosseratStressBase(const InputParameters & parameters)
  : ComputeStressBase(parameters),
    _curvature(getMaterialPropertyByName<RankTwoTensor>("curvature")),
    _elastic_flexural_rigidity_tensor(
        getMaterialPropertyByName<RankFourTensor>("elastic_flexural_rigidity_tensor")),
    _stress_couple(declareProperty<RankTwoTensor>("couple_stress")),
    _Jacobian_mult_couple(declareProperty<RankFourTensor>("couple_Jacobian_mult"))
{
}

void
ComputeCosseratStressBase::initQpStatefulProperties()
{
  ComputeStressBase::initQpStatefulProperties();
  _stress_couple[_qp].zero();
}
