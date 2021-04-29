//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MatKernel.h"

registerMooseObject("MooseTestApp", MatKernel);

InputParameters
MatKernel::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addParam<Real>("coefficient", 1.0, "Coefficient of the term");
  params.addParam<MaterialPropertyName>("mat_prop",
                                        "the name of the material property for coupling");
  return params;
}

MatKernel::MatKernel(const InputParameters & parameters)
  : Kernel(parameters),
    _mat_prop(getMaterialProperty<Real>("mat_prop")),
    _coef(getParam<Real>("coefficient"))
{
}

Real
MatKernel::computeQpResidual()
{
  return _coef * _test[_i][_qp] * _mat_prop[_qp];
}

// we currently have no way to assemble Jacobian properly for a general material property
