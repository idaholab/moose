//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFEFluidMomentumBC.h"

registerMooseObject("NavierStokesApp", INSFEFluidMomentumBC);
registerMooseObjectRenamed("NavierStokesApp",
                           MDFluidMomentumBC,
                           "02/01/2024 00:00",
                           INSFEFluidMomentumBC);

InputParameters
INSFEFluidMomentumBC::validParams()
{
  InputParameters params = INSFEFluidIntegratedBCBase::validParams();
  params.addClassDescription("Specifies flow of momentum through a boundary");
  params.addParam<bool>("conservative_form", false, "Whether the conservative form is used");
  params.addParam<bool>(
      "p_int_by_parts", false, "Whether integration by parts is applied to the pressure term");
  params.addRequiredParam<unsigned>("component", "0,1,or 2 for x-, y-, or z- direction");
  params.addParam<FunctionName>("p_fn", "Pressure function with time at the boundary");
  params.addParam<FunctionName>("v_fn", "Velocity function with time at the boundary");

  // coupled with branch pressure and density
  // The 'branch_center' is a little bit tricky, because SAM 1D and multi-D could be in
  // different mesh system.
  //   * The volume branch center is always defined in physical 3D XYZ coordinate system,
  //   * but multi-D flow could be simulated in 2D XY coordinate system,
  //   * the volume brance center needs be mapped to the 2D/3D flow mesh system
  // The pressure at the multi-D boundary and the branch pressure is related by:
  //   p_boundary = p_branch + rho_branch * (point_boundary - branch_center) * gravity
  params.addCoupledVar("p_branch", "Coupled scalar branch pressure");
  params.addCoupledVar("rho_branch", "Coupled scalar branch density for gravity head calculation");
  params.addParam<VectorValue<Real>>("gravity", "Gravity vector in 2D/3D flow mesh system");
  params.addParam<Point>("branch_center", "Position of branch center in 2D/3D flow mesh system");
  return params;
}

INSFEFluidMomentumBC::INSFEFluidMomentumBC(const InputParameters & parameters)
  : INSFEFluidIntegratedBCBase(parameters),
    _conservative_form(getParam<bool>("conservative_form")),
    _p_int_by_parts(getParam<bool>("p_int_by_parts")),
    _component(getParam<unsigned>("component")),
    _mu(getMaterialProperty<Real>("dynamic_viscosity")),
    _mu_t(getMaterialProperty<Real>("turbulence_viscosity")),
    _has_pbc(parameters.isParamValid("p_fn")),
    _has_vbc(parameters.isParamValid("v_fn")),
    _p_fn(_has_pbc ? &getFunction("p_fn") : nullptr),
    _v_fn(_has_vbc ? &getFunction("v_fn") : nullptr),
    _has_pbranch(parameters.isParamValid("p_branch")),
    _p_branch(_has_pbranch ? coupledScalarValue("p_branch") : _zero),
    _p_branch_var_number(_has_pbranch ? coupledScalar("p_branch") : libMesh::invalid_uint),
    _rho_branch(_has_pbranch ? coupledScalarValue("rho_branch") : _zero)
{
  if ((_has_pbc || _has_pbranch) && _has_vbc)
    mooseError("Pressure and velocity cannot be BOTH specified in INSFEFluidMomentumBC.");
  //
  if (_has_pbc && _has_pbranch)
    mooseError(
        "Pressure function and branch pressure cannot be BOTH specified in INSFEFluidMomentumBC.");
  //
  if (_has_pbranch)
  {
    if (!(isParamValid("gravity") && isParamValid("branch_center")))
    {
      mooseError(
          name(),
          ": this boundary is coupled to a volume branch, ",
          "please provide 'gravity' vector and 'branch_center' for gravity head calculation.");
    }
    _vec_g = getParam<VectorValue<Real>>("gravity");
    _branch_center = getParam<Point>("branch_center");
  }
}

Real
INSFEFluidMomentumBC::computeQpResidual()
{
  Real porosity = _has_porosity ? _porosity[_qp] : 1;
  RealVectorValue vec_vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // pressure bc value
  Real p_bc = 0.0;
  if (_has_pbc)
    p_bc = _p_fn->value(_t, _q_point[_qp]);
  else if (_has_pbranch)
  {
    Real dH = (_q_point[_qp] - _branch_center) * _vec_g;
    p_bc = _p_branch[0] + _rho_branch[0] * dH;
  }
  else
    p_bc = _pressure[_qp];

  // velocity bc value
  Real v_bc = _has_vbc ? -_v_fn->value(_t, _q_point[_qp]) : vec_vel * _normals[_qp];

  Real viscous_part = (porosity > 0.99)
                          ? -(_mu[_qp] + _mu_t[_qp]) * _grad_u[_qp] * _normals[_qp] * _test[_i][_qp]
                          : 0;
  Real p_part = _p_int_by_parts ? porosity * p_bc * _normals[_qp](_component) * _test[_i][_qp] : 0;
  Real conv_part = _conservative_form ? _rho[_qp] * _u[_qp] * v_bc / porosity * _test[_i][_qp] : 0;

  return p_part + conv_part + viscous_part;
}

Real
INSFEFluidMomentumBC::computeQpJacobian()
{
  Real porosity = _has_porosity ? _porosity[_qp] : 1;
  RealVectorValue vec_vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  Real v_bc = _has_vbc ? -_v_fn->value(_t, _q_point[_qp]) : vec_vel * _normals[_qp];

  Real conv_part = 0;
  if (_conservative_form)
  {
    if (_has_vbc)
      conv_part = _rho[_qp] * _phi[_j][_qp] * v_bc / porosity * _test[_i][_qp];
    else
      conv_part = _rho[_qp] * _phi[_j][_qp] * v_bc / porosity * _test[_i][_qp] +
                  _rho[_qp] * _u[_qp] * _phi[_j][_qp] * _normals[_qp](_component) / porosity *
                      _test[_i][_qp];
  }
  Real viscous_part = (porosity > 0.99)
                          ? -(_mu[_qp] + _mu_t[_qp]) * _grad_phi[_j][_qp](_component) *
                                _normals[_qp](_component) * _test[_i][_qp]
                          : 0;

  return conv_part + viscous_part;
}

Real
INSFEFluidMomentumBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  unsigned m = this->mapVarNumber(jvar);
  RealVectorValue vec_vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  Real porosity = _has_porosity ? _porosity[_qp] : 1;

  // this is the jacobian w.r.t branch pressure
  if (jvar == _p_branch_var_number)
    return _p_int_by_parts ? porosity * _normals[_qp](_component) * _test[_i][_qp] : 0;

  Real jac = 0;
  switch (m)
  {
    case 0:
      if (!(_has_pbc || _has_pbranch))
        jac = _p_int_by_parts
                  ? porosity * _phi[_j][_qp] * _normals[_qp](_component) * _test[_i][_qp]
                  : 0;
      break;

    case 1:
    case 2:
    case 3:
      if (m != (_component + 1))
        jac = _conservative_form ? _rho[_qp] / porosity * _phi[_j][_qp] * _test[_i][_qp] * _u[_qp] *
                                       _normals[_qp](m - 1)
                                 : 0;
      break;

    case 4:
    default:
      jac = 0;
  }

  return jac;
}
