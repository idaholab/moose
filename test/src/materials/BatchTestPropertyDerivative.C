//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BatchTestPropertyDerivative.h"

registerMooseObject("MooseTestApp", BatchTestPropertyDerivative);

InputParameters
BatchTestPropertyDerivative::validParams()
{
  // we couple the same data as the BatchMaterialTest UO to compare the results of
  // batch and regular computation
  auto params = Material::validParams();
  params.addRequiredParam<MaterialPropertyName>("prop", "A RankTwoTensor property");
  params.addRequiredParam<UserObjectName>("batch_uo",
                                          "User object that performs the batch computation");
  return params;
}

BatchTestPropertyDerivative::BatchTestPropertyDerivative(const InputParameters & parameters)
  : Material(parameters),
    _prop(getMaterialProperty<RankTwoTensor>("prop")),
    _prop_out(declareProperty<Real>("batch_out")),
    _batch_uo(getUserObject<BatchPropertyDerivativeTest>("batch_uo")),
    _output(_batch_uo.getOutputData())
{
}

void
BatchTestPropertyDerivative::computeProperties()
{
  if (!_batch_uo.outputReady())
    return;

  const auto index = _batch_uo.getIndex(_current_elem->id());

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    // compute value
    const Real result_conventional = _prop[_qp].norm();

    // output result
    _prop_out[_qp] = _output[index + _qp];

    // check that we got the same value from batch material UO
    if (result_conventional != _output[index + _qp])
      mooseError("Welp, this didn't work as expected! ",
                 result_conventional,
                 " != ",
                 _output[index + _qp]);
  }
}
