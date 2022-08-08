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
    _prop_out(declareProperty<Real>("batch_out")),
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
    const Real result_conventional = _var1[_qp] * _prop1[_qp].L2norm() + _prop2[_qp];

    // get the batch compute result
    const Real result_batch = _output[index + _qp];

    // output result
    _prop_out[_qp] = result_batch;

    // check that we got the same value
    if (result_conventional != result_batch)
      mooseError("Welp, this didn't work as expected! ", result_conventional, " != ", result_batch);
  }
}
