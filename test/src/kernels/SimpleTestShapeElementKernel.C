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

#include "SimpleTestShapeElementKernel.h"

template <>
InputParameters
validParams<SimpleTestShapeElementKernel>()
{
  InputParameters params = validParams<NonlocalKernel>();
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
