/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "ExampleShapeElementKernel.h"

template <>
InputParameters
validParams<ExampleShapeElementKernel>()
{
  InputParameters params = validParams<NonlocalKernel>();
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
