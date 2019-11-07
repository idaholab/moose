//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialEigenKernel.h"

registerMooseObject("MooseTestApp", MaterialEigenKernel);

InputParameters
MaterialEigenKernel::validParams()
{
  InputParameters params = EigenKernel::validParams();
  params.addParam<MaterialPropertyName>("mat", "Material property name (pseudo-stateful)");
  return params;
}

MaterialEigenKernel::MaterialEigenKernel(const InputParameters & parameters)
  : EigenKernel(parameters),
    _propname(getParam<MaterialPropertyName>("mat")),
    _mat(_is_implicit ? getMaterialPropertyByName<Real>(_propname)
                      : getMaterialPropertyByName<Real>(_propname + "_old"))
{
}

Real
MaterialEigenKernel::computeQpResidual()
{
  return -_mat[_qp] * _test[_i][_qp];
}
