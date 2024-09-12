//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLagrangianObjectiveCustomStress.h"

registerMooseObject("SolidMechanicsApp", ComputeLagrangianObjectiveCustomStress);
registerMooseObject("SolidMechanicsApp", ComputeLagrangianObjectiveCustomSymmetricStress);

template <bool symmetric>
InputParameters
ComputeLagrangianObjectiveCustomStressTmpl<symmetric>::validParams()
{
  InputParameters params = ComputeLagrangianObjectiveStress::validParams();
  params.addRequiredParam<MaterialPropertyName>("custom_small_stress",
                                                "The name of the small stress.");
  params.addRequiredParam<MaterialPropertyName>("custom_small_jacobian",
                                                "The name of the small stress Jacobian.");
  return params;
}

template <bool symmetric>
ComputeLagrangianObjectiveCustomStressTmpl<symmetric>::ComputeLagrangianObjectiveCustomStressTmpl(
    const InputParameters & parameters)
  : ComputeLagrangianObjectiveStress(parameters),
    _custom_stress(getMaterialProperty<StressType>("custom_small_stress")),
    _custom_jacobian(getMaterialProperty<JacobianType>("custom_small_jacobian"))
{
}

template <bool symmetric>
void
ComputeLagrangianObjectiveCustomStressTmpl<symmetric>::computeQpSmallStress()
{
  _small_stress[_qp] = RankTwoTensor(_custom_stress[_qp]);
  _small_jacobian[_qp] = RankFourTensor(_custom_jacobian[_qp]);
}
