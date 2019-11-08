//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SimpleTestShapeElementKernel.h"

registerMooseObject("MooseTestApp", SimpleTestShapeElementKernel);

InputParameters
SimpleTestShapeElementKernel::validParams()
{
  InputParameters params = NonlocalKernel::validParams();
  params.addRequiredParam<UserObjectName>("user_object",
                                          "Name of a SimpleTestShapeElementUserObject");
  return params;
}

SimpleTestShapeElementKernel::SimpleTestShapeElementKernel(const InputParameters & parameters)
  : NonlocalKernel(parameters),
    _shp(getUserObject<SimpleTestShapeElementUserObject>("user_object")),
    _shp_integral(_shp.getIntegral()),
    _shp_jacobian(_shp.getJacobian()),
    _var_dofs(_var.dofIndices())
{
}

Real
SimpleTestShapeElementKernel::computeQpResidual()
{
  return _test[_i][_qp] * _shp_integral;
}

Real
SimpleTestShapeElementKernel::computeQpJacobian()
{
  return _test[_i][_qp] * _shp_jacobian[_var_dofs[_j]];
}

Real
SimpleTestShapeElementKernel::computeQpNonlocalJacobian(dof_id_type dof_index)
{
  return _test[_i][_qp] * _shp_jacobian[dof_index];
}
