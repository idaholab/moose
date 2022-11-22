//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MDFluidEnergyBC.h"

registerMooseObject("NavierStokesApp", MDFluidEnergyBC);

InputParameters
MDFluidEnergyBC::validParams()
{
  InputParameters params = MDFluidIntegratedBCBase::validParams();
  params.addParam<FunctionName>("v_fn", "Velocity function with time at the boundary");
  params.addParam<FunctionName>("T_fn", "Temperature function with time at the boundary");
  params.addCoupledVar("porosity_elem", "Element averaged porosity");
  params.addCoupledVar("T_branch", "Coupled scalar branch temperature");
  return params;
}

MDFluidEnergyBC::MDFluidEnergyBC(const InputParameters & parameters)
  : MDFluidIntegratedBCBase(parameters),
    _cp(getMaterialProperty<Real>("cp_fluid")),
    _has_vbc(parameters.isParamValid("v_fn")),
    _has_Tbc(parameters.isParamValid("T_fn")),
    _v_fn(_has_vbc ? &getFunction("v_fn") : NULL),
    _T_fn(_has_Tbc ? &getFunction("T_fn") : NULL),
    _k_elem(getMaterialProperty<Real>("k_fluid_elem")),
    _has_porosity_elem(isParamValid("porosity_elem")),
    _porosity_elem(_has_porosity_elem ? coupledValue("porosity_elem")
                                      : (_has_porosity ? coupledValue("porosity") : _zero)),
    _has_Tbranch(parameters.isParamValid("T_branch")),
    _T_branch(_has_Tbranch ? coupledScalarValue("T_branch") : _zero),
    _T_branch_var_number(_has_Tbranch ? coupledScalar("T_branch") : libMesh::invalid_uint)
{
  if (_has_vbc && !_has_Tbc)
    mooseError("For an inlet condition ('v_fn' is given), a boundary temperature ('T_fn') is also "
               "needed.");
  //
  if (_has_Tbc && _has_Tbranch)
    mooseError(
        "Temperature function and branch temperature cannot be BOTH specified in MDFluidEnergyBC.");
}

Real
MDFluidEnergyBC::computeQpResidual()
{
  RealVectorValue vec_vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  Real v_bc = _has_vbc ? -_v_fn->value(_t, _q_point[_qp]) : vec_vel * _normals[_qp];
  Real T_bc = _u[_qp];
  // A more restrict condition might be needed here
  // For an inlet or reverse outlet flow condition, a T_bc or a T_branch should be required.
  if (v_bc < 0)
  {
    if (_has_Tbc)
      T_bc = _T_fn->value(_t, _q_point[_qp]);
    if (_has_Tbranch)
      T_bc = _T_branch[0];
  }
  Real enthalpy = _eos.h_from_p_T(_pressure[_qp], T_bc);

  Real convection_term = _rho[_qp] * v_bc * enthalpy * _test[_i][_qp];
  Real diffusion_term =
      -_porosity_elem[_qp] * _k_elem[_qp] * _grad_u[_qp] * _normals[_qp] * _test[_i][_qp];

  return convection_term + diffusion_term;
}

Real
MDFluidEnergyBC::computeQpJacobian()
{
  RealVectorValue vec_vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  Real v_bc = _has_vbc ? -_v_fn->value(_t, _q_point[_qp]) : vec_vel * _normals[_qp];

  Real convection_term = 0;
  if (v_bc < 0) // Inlet
    convection_term = 0;
  else // Outlet or not a real boundary
  {
    Real rho, drho_dp, drho_dT;
    _eos.rho_from_p_T(_pressure[_qp], _u[_qp], rho, drho_dp, drho_dT);
    Real enthalpy = _eos.h_from_p_T(_pressure[_qp], _u[_qp]);

    convection_term =
        (_rho[_qp] * _cp[_qp] + drho_dT * enthalpy) * v_bc * _phi[_j][_qp] * _test[_i][_qp];
  }

  Real diffusion_term =
      -_porosity_elem[_qp] * _k_elem[_qp] * _grad_phi[_j][_qp] * _normals[_qp] * _test[_i][_qp];

  return convection_term + diffusion_term;
}

Real
MDFluidEnergyBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  unsigned m = this->map_var_number(jvar);

  // this is jocabian term w.r.t branch temperature
  if (jvar == _T_branch_var_number)
  {
    RealVectorValue vec_vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
    Real v_bc = _has_vbc ? -_v_fn->value(_t, _q_point[_qp]) : vec_vel * _normals[_qp];
    if (v_bc < 0)
    {
      return _rho[_qp] * v_bc * _eos.cp_from_p_T(1e5, _T_branch[0]) * _test[_i][_qp];
    }
  }
  switch (m)
  {
    case 1:
    case 2:
    case 3:
      if (!_has_vbc) // if has_vbc, Jacobians are zero
      {
        RealVectorValue vec_vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
        Real v_bc = vec_vel * _normals[_qp];
        Real T_bc = _u[_qp];
        if (v_bc < 0)
        {
          if (_has_Tbc)
            T_bc = _T_fn->value(_t, _q_point[_qp]);
          if (_has_Tbranch)
            T_bc = _T_branch[0];
        }
        Real enthalpy = _eos.h_from_p_T(_pressure[_qp], T_bc);
        return _rho[_qp] * _phi[_j][_qp] * enthalpy * _normals[_qp](m - 1) * _test[_i][_qp];
      }

    default:
      return 0;
  }
}
