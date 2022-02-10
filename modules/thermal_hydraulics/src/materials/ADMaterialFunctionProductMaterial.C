//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADMaterialFunctionProductMaterial.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", ADMaterialFunctionProductMaterial);

InputParameters
ADMaterialFunctionProductMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addRequiredParam<MaterialPropertyName>("mat_prop_product", "Product material property");
  params.addRequiredParam<MaterialPropertyName>("mat_prop_scale", "Scale material property");
  params.addRequiredParam<FunctionName>("function", "Function in product");

  params.addClassDescription("Computes the product of a material property and a function.");

  return params;
}

ADMaterialFunctionProductMaterial::ADMaterialFunctionProductMaterial(
    const InputParameters & parameters)
  : Material(parameters),

    _product(declareADProperty<Real>(getParam<MaterialPropertyName>("mat_prop_product"))),
    _scale(getADMaterialProperty<Real>("mat_prop_scale")),
    _function(getFunction("function"))
{
}

void
ADMaterialFunctionProductMaterial::computeQpProperties()
{
  _product[_qp] = _scale[_qp] * _function.value(_t, _q_point[_qp]);
}
