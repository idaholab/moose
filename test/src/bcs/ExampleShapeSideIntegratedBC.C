//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExampleShapeSideIntegratedBC.h"

registerMooseObject("MooseTestApp", ExampleShapeSideIntegratedBC);

InputParameters
ExampleShapeSideIntegratedBC::validParams()
{
  InputParameters params = NonlocalIntegratedBC::validParams();
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
