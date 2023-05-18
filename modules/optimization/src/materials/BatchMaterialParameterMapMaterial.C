//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BatchMaterialParameterMapMaterial.h"

registerMooseObject("OptimizationApp", BatchMaterialParameterMapMaterial);

InputParameters
BatchMaterialParameterMapMaterial::validParams()
{
  // we couple the same data as the BatchMaterialParameterMap UO to compare the results of
  // batch and regular computation
  auto params = Material::validParams();
  params.addRequiredParam<MaterialPropertyName>("prop1", "A Real property");
  params.addRequiredParam<UserObjectName>("batch_uo",
                                          "User object that performs the batch computation");
  params.addRequiredParam<MaterialName>("batch_material_name",
                                        "Name given to the batch material property created.");
  return params;
}

BatchMaterialParameterMapMaterial::BatchMaterialParameterMapMaterial(
    const InputParameters & parameters)
  : Material(parameters),
    _prop1(getMaterialProperty<Real>("prop1")),
    _prop_out(declareProperty<Real>(getParam<MaterialName>("batch_material_name"))),
    _batch_uo(getUserObject<BatchMaterialParameterMap>("batch_uo")),
    _output(_batch_uo.getOutputData())
{
}

void
BatchMaterialParameterMapMaterial::computeProperties()
{
  if (!_batch_uo.outputReady())
    return;

  const auto index = _batch_uo.getIndex(_current_elem->id());

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    // compute value the old fashioned way
    const Real result_conventional = _prop1[_qp];

    // get the batch compute result
    const Real result_batch = _output[index + _qp];

    // output result
    _prop_out[_qp] = result_batch;

    // check that we got the same value
    if (result_conventional != result_batch)
      mooseError("Mismatch between material property and batch material output: ",
                 result_conventional,
                 " != ",
                 result_batch);
  }
}
