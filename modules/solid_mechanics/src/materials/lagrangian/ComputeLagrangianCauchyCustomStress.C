//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLagrangianCauchyCustomStress.h"

registerMooseObject("SolidMechanicsApp", ComputeLagrangianCauchyCustomStress);

InputParameters
ComputeLagrangianCauchyCustomStress::validParams()
{
  InputParameters params = ComputeLagrangianStressCauchy::validParams();

  params.addClassDescription("Custom stress update providing the Cauchy stress");

  params.addRequiredParam<MaterialPropertyName>("custom_cauchy_stress",
                                                "The name of the Cauchy stress.");
  params.addRequiredParam<MaterialPropertyName>("custom_cauchy_jacobian",
                                                "The name of the Cauchy stress Jacobian.");

  return params;
}

ComputeLagrangianCauchyCustomStress::ComputeLagrangianCauchyCustomStress(
    const InputParameters & parameters)
  : ComputeLagrangianStressCauchy(parameters),
    _custom_stress(getMaterialProperty<RankTwoTensor>("custom_cauchy_stress")),
    _custom_jacobian(getMaterialProperty<RankFourTensor>("custom_cauchy_jacobian"))
{
}

void
ComputeLagrangianCauchyCustomStress::computeQpCauchyStress()
{
  _cauchy_stress[_qp] = _custom_stress[_qp];
  _cauchy_jacobian[_qp] = _custom_jacobian[_qp];
}
