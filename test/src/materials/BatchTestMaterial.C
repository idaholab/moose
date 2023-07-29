//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BatchTestMaterial.h"

registerMooseObject("MooseTestApp", BatchTestMaterial);

InputParameters
BatchTestMaterial::validParams()
{
  // we couple the same data as the BatchMaterialTest UO to compare the results of
  // batch and regular computation
  auto params = Material::validParams();
  params.addCoupledVar("var1", "A coupled variable");
  params.addRequiredParam<MaterialPropertyName>("prop1", "A RankTwoTensor property");
  params.addRequiredParam<MaterialPropertyName>("prop2", "A Real property");
  params.addRequiredParam<UserObjectName>("batch_uo",
                                          "User object that performs the batch computation");
  return params;
}

BatchTestMaterial::BatchTestMaterial(const InputParameters & parameters)
  : Material(parameters),
    _var1(coupledValue("var1")),
    _prop1(getMaterialProperty<RankTwoTensor>("prop1")),
    _prop2(getMaterialProperty<Real>("prop2")),
    _var1_n(coupledValueOld("var1")),
    _prop1_n(getMaterialPropertyOld<RankTwoTensor>("prop1")),
    _prop2_n(getMaterialPropertyOld<Real>("prop2")),
    _prop_out1(declareProperty<Real>("batch_out1")),
    _prop_out2(declareProperty<Real>("batch_out2")),
    _batch_uo(getUserObject<BatchMaterialTest>("batch_uo")),
    _output(_batch_uo.getOutputData())
{
}

void
BatchTestMaterial::computeProperties()
{
  if (!_batch_uo.outputReady())
    return;

  const auto index = _batch_uo.getIndex(_current_elem->id());

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    // compute value the old fashioned way
    const Real result_conventional_1 = 5 * (_var1[_qp] * _prop1[_qp].L2norm() + _prop2[_qp]);
    const Real result_conventional_2 = 3 * (_var1_n[_qp] - _prop1_n[_qp].trace() * _prop2_n[_qp]);

    // get the batch compute result
    const auto [result_batch_1, result_batch_2] = _output[index + _qp];

    // output result
    _prop_out1[_qp] = result_batch_1;
    _prop_out2[_qp] = result_batch_2;

    // check that we got the same value
    if (result_conventional_1 != result_batch_1)
      mooseError(
          "Welp, this didn't work as expected! 1: ", result_conventional_1, " != ", result_batch_1);
    if (result_conventional_2 != result_batch_2)
      mooseError(
          "Welp, this didn't work as expected! 2: ", result_conventional_2, " != ", result_batch_2);
  }
}
