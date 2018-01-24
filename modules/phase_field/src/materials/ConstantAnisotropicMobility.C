//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConstantAnisotropicMobility.h"

template <>
InputParameters
validParams<ConstantAnisotropicMobility>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("Provide a constant mobility tensor value");
  params.addRequiredParam<MaterialPropertyName>("M_name",
                                                "Name of the mobility tensor porperty to generate");
  params.addRequiredRangeCheckedParam<std::vector<Real>>(
      "tensor", "tensor_size=9", "Tensor values");
  return params;
}

ConstantAnisotropicMobility::ConstantAnisotropicMobility(const InputParameters & parameters)
  : Material(parameters),
    _M_values(getParam<std::vector<Real>>("tensor")),
    _M_name(getParam<MaterialPropertyName>("M_name")),
    _M(declareProperty<RealTensorValue>(_M_name))
{
}

void
ConstantAnisotropicMobility::initialSetup()
{
  _M.resize(_fe_problem.getMaxQps());
  for (unsigned int qp = 0; qp < _M.size(); ++qp)
    for (unsigned int a = 0; a < LIBMESH_DIM; ++a)
      for (unsigned int b = 0; b < LIBMESH_DIM; ++b)
        _M[qp](a, b) = _M_values[a * 3 + b];
}
