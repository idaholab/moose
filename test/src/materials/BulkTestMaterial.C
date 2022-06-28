//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BulkTestMaterial.h"

registerMooseObject("MooseTestApp", BulkTestMaterial);

InputParameters
BulkTestMaterial::validParams()
{
  // we couple the same data as the BulkMaterialTest UO to compare the results of
  // bulk and regular computation
  auto params = Material::validParams();
  params.addCoupledVar("var1", "A coupled variable");
  params.addRequiredParam<MaterialPropertyName>("prop1", "A RankTwoTensor property");
  params.addRequiredParam<MaterialPropertyName>("prop2", "A Real property");
  params.addRequiredParam<UserObjectName>("bulk_uo",
                                          "User object that performs the bulk computation");
  return params;
}

BulkTestMaterial::BulkTestMaterial(const InputParameters & parameters)
  : Material(parameters),
    _var1(coupledValue("var1")),
    _prop1(getMaterialProperty<RankTwoTensor>("prop1")),
    _prop2(getMaterialProperty<Real>("prop2")),
    _prop_out(declareProperty<Real>("bulk_out")),
    _bulk_uo(getUserObject<BulkMaterialTest>("bulk_uo")),
    _output(_bulk_uo.getOutputData())
{
}

void
BulkTestMaterial::computeProperties()
{
  const auto index = _bulk_uo.getIndex(_current_elem->id());

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    // compute value the old fashioned way
    const Real result_conventional = _var1[_qp] * _prop1[_qp].L2norm() + _prop2[_qp];

    // get the bulk compute result
    const Real result_bulk = _output[index + _qp];

    // output result
    _prop_out[_qp] = result_bulk;

    // check that we got the same value
    if (result_conventional != result_bulk)
      mooseError("Welp, this didn't work as expected! ", result_conventional, " != ", result_bulk);
  }
}
