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

#include "ExampleShapeSideIntegratedBC.h"

template <>
InputParameters
validParams<ExampleShapeSideIntegratedBC>()
{
  InputParameters params = validParams<NonlocalIntegratedBC>();
  params.addRequiredParam<UserObjectName>(
      "num_user_object", "ShapeSideUserObject for computing integral component in numerator.");
  params.addRequiredParam<UserObjectName>(
      "denom_user_object", "ShapeSideUserObject for computing integral component in denominator.");
  params.addRequiredCoupledVar("v", "Charge species.");
  params.addRequiredParam<Real>("Vb", "Initial potential applied to the boundary.");
  return params;
}

ExampleShapeSideIntegratedBC::ExampleShapeSideIntegratedBC(const InputParameters & parameters)
  : NonlocalIntegratedBC(parameters),
    _num_shp(getUserObject<NumShapeSideUserObject>("num_user_object")),
    _num_shp_integral(_num_shp.getIntegral()),
    _num_shp_jacobian(_num_shp.getJacobian()),
    _denom_shp(getUserObject<DenomShapeSideUserObject>("denom_user_object")),
    _denom_shp_integral(_denom_shp.getIntegral()),
    _denom_shp_jacobian(_denom_shp.getJacobian()),
    _var_dofs(_var.dofIndices()),
    _v_var(coupled("v")),
    _v_dofs(getVar("v", 0)->dofIndices()),
    _Vb(getParam<Real>("Vb"))
{
}

Real
ExampleShapeSideIntegratedBC::computeQpResidual()
{
  return _test[_i][_qp] * (_u[_qp] + _num_shp_integral - _Vb) /
         (_denom_shp_integral + std::numeric_limits<Real>::epsilon());
}

Real
ExampleShapeSideIntegratedBC::computeQpJacobian()
{
  return _test[_i][_qp] * (_phi[_j][_qp]) /
         (_denom_shp_integral + std::numeric_limits<Real>::epsilon());
}

Real
ExampleShapeSideIntegratedBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _v_var)
  {
    Real jac = _test[_i][_qp] *
               (_num_shp_jacobian[_v_dofs[_j]] * _denom_shp_integral -
                (_u[_qp] + _num_shp_integral - _Vb) * _denom_shp_jacobian[_v_dofs[_j]]) /
               (_denom_shp_integral * _denom_shp_integral + std::numeric_limits<Real>::epsilon());
    return jac;
  }

  return 0.0;
}

Real ExampleShapeSideIntegratedBC::computeQpNonlocalJacobian(dof_id_type /*dof_index*/)
{
  return 0;
}

Real
ExampleShapeSideIntegratedBC::computeQpNonlocalOffDiagJacobian(unsigned int jvar,
                                                               dof_id_type dof_index)
{
  if (jvar == _v_var)
  {
    Real jac = _test[_i][_qp] *
               (_num_shp_jacobian[dof_index] * _denom_shp_integral -
                (_u[_qp] + _num_shp_integral - _Vb) * _denom_shp_jacobian[dof_index]) /
               (_denom_shp_integral * _denom_shp_integral + std::numeric_limits<Real>::epsilon());
    return jac;
  }

  return 0.0;
}
