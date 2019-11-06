//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExampleShapeElementKernel.h"

registerMooseObject("MooseTestApp", ExampleShapeElementKernel);

InputParameters
ExampleShapeElementKernel::validParams()
{
  InputParameters params = NonlocalKernel::validParams();
  params.addRequiredParam<UserObjectName>("user_object",
                                          "Name of an ExampleShapeElementUserObject");
  params.addRequiredCoupledVar("v", "coupled variable");
  return params;
}

ExampleShapeElementKernel::ExampleShapeElementKernel(const InputParameters & parameters)
  : NonlocalKernel(parameters),
    _shp(getUserObject<ExampleShapeElementUserObject>("user_object")),
    _shp_integral(_shp.getIntegral()),
    _shp_jacobian(_shp.getJacobian()),
    _var_dofs(_var.dofIndices()),
    _v_var(coupled("v")),
    _v_dofs(getVar("v", 0)->dofIndices())
{
}

Real
ExampleShapeElementKernel::computeQpResidual()
{
  return _test[_i][_qp] * _shp_integral;
}

Real
ExampleShapeElementKernel::computeQpJacobian()
{
  return _test[_i][_qp] * _shp_jacobian[_var_dofs[_j]];
}

Real
ExampleShapeElementKernel::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _v_var)
    return _test[_i][_qp] * _shp_jacobian[_v_dofs[_j]];

  return 0.0;
}

Real
ExampleShapeElementKernel::computeQpNonlocalJacobian(dof_id_type dof_index)
{
  return _test[_i][_qp] * _shp_jacobian[dof_index];
}

Real
ExampleShapeElementKernel::computeQpNonlocalOffDiagJacobian(unsigned int jvar,
                                                            dof_id_type dof_index)
{
  if (jvar == _v_var)
    return _test[_i][_qp] * _shp_jacobian[dof_index];

  return 0.0;
}
