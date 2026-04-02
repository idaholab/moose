//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledValueFunctionTensorMaterial.h"
#include <cstdlib>

registerMooseObject("MooseApp", CoupledValueFunctionTensorMaterial);

InputParameters
CoupledValueFunctionTensorMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Assembles a RankTwoTensor from scalar material properties or constants");
  params.addParam<MaterialPropertyName>("tensor_name", "Name of output tensor");
  params.addRequiredParam<std::vector<std::string>>(
      "tensor_values",
      "9 tensor components (row-major) as material property names or constants");
  return params;
}

CoupledValueFunctionTensorMaterial::CoupledValueFunctionTensorMaterial(
    const InputParameters & parameters)
  : Material(parameters),
    _prop(declareProperty<RankTwoTensor>(getParam<MaterialPropertyName>("tensor_name")))
{
  const auto & vals = getParam<std::vector<std::string>>("tensor_values");
  if (vals.size() != 9)
    paramError("tensor_values", "Must provide exactly 9 values");

  _mat_props.resize(9, nullptr);
  _const_vals.resize(9);
  _is_const.resize(9);

  for (unsigned int i = 0; i < 9; ++i)
  {
    char * end;
    double val = std::strtod(vals[i].c_str(), &end);
    

    if (end != vals[i].c_str() && *end == '\0')
    {
      _const_vals[i] = val;
      _is_const[i] = true;
    }
    else
    {
      _mat_props[i] = &getMaterialProperty<Real>(vals[i]);
      _is_const[i] = false;
    }
  }
}

void
CoupledValueFunctionTensorMaterial::computeQpProperties()
{
  unsigned int k = 0;
  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
    {
      _prop[_qp](i, j) = _is_const[k] ? _const_vals[k] : (*_mat_props[k])[_qp];
      k++;
    }
}