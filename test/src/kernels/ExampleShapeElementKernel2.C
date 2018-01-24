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

#include "ExampleShapeElementKernel2.h"

template <>
InputParameters
validParams<ExampleShapeElementKernel2>()
{
  InputParameters params = validParams<NonlocalKernel>();
  params.addRequiredParam<UserObjectName>("user_object",
                                          "Name of an ExampleShapeElementUserObject");
  params.addRequiredCoupledVar("u", "coupled variable");
  params.addRequiredCoupledVar("v", "second coupled variable");
  return params;
}

ExampleShapeElementKernel2::ExampleShapeElementKernel2(const InputParameters & parameters)
  : NonlocalKernel(parameters),
    _shp(getUserObject<ExampleShapeElementUserObject>("user_object")),
    _shp_integral(_shp.getIntegral()),
    _shp_jacobian(_shp.getJacobian()),
    _u_var(coupled("u")),
    _u_dofs(getVar("u", 0)->dofIndices()),
    _v_var(coupled("v")),
    _v_dofs(getVar("v", 0)->dofIndices())
{
}

Real
ExampleShapeElementKernel2::computeQpResidual()
{
  return _test[_i][_qp] * _shp_integral;
}

Real
ExampleShapeElementKernel2::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _u_var)
    return _test[_i][_qp] * _shp_jacobian[_u_dofs[_j]];

  if (jvar == _v_var)
    return _test[_i][_qp] * _shp_jacobian[_v_dofs[_j]];

  return 0.0;
}

Real
ExampleShapeElementKernel2::computeQpNonlocalOffDiagJacobian(unsigned int jvar,
                                                             dof_id_type dof_index)
{
  if (jvar == _u_var)
    return _test[_i][_qp] * _shp_jacobian[dof_index];

  if (jvar == _v_var)
    return _test[_i][_qp] * _shp_jacobian[dof_index];

  return 0.0;
}
