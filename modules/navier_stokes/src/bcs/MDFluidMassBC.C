//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MDFluidMassBC.h"

registerMooseObject("NavierStokesApp", MDFluidMassBC);

InputParameters
MDFluidMassBC::validParams()
{
  InputParameters params = MDFluidIntegratedBCBase::validParams();
  params.addParam<FunctionName>("v_fn", "Velocity function with time at the boundary");
  params.addParam<std::string>("v_pps",
                               "The Postprocessor name to setup the velocity boundary value.");

  return params;
}

MDFluidMassBC::MDFluidMassBC(const InputParameters & parameters)
  : MDFluidIntegratedBCBase(parameters),
    _has_vfn(parameters.isParamValid("v_fn")),
    _has_vpps(parameters.isParamValid("v_pps")),
    _velocity_fn(_has_vfn ? &getFunction("v_fn") : NULL),
    _v_pps_name(_has_vpps ? getParam<std::string>("v_pps") : "")
{
  if (_has_vfn && _has_vpps)
    mooseError("'v_fn' and 'v_pps' cannot be BOTH specified in MDFluidMassBC.");
}

Real
MDFluidMassBC::computeQpResidual()
{
  RealVectorValue vec_vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  Real v_bc = 0;
  if (_has_vfn)
    v_bc = -_velocity_fn->value(_t, _q_point[_qp]); // normals[i] at inlet is negative.
  else if (_has_vpps)
    v_bc = -getPostprocessorValueByName(_v_pps_name);
  else
    v_bc = vec_vel * _normals[_qp];

  return _rho[_qp] * v_bc * _test[_i][_qp];
}

Real
MDFluidMassBC::computeQpJacobian()
{
  return 0;
}

Real
MDFluidMassBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  unsigned m = this->map_var_number(jvar);

  switch (m)
  {
    case 1:
    case 2:
    case 3:
    {
      if (_has_vfn || _has_vpps)
        return 0.;
      else
        return _rho[_qp] * _phi[_j][_qp] * _normals[_qp](m - 1) * _test[_i][_qp];
    }

    case 4:
    {
      RealVectorValue vec_vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
      Real rho, drho_dp, drho_dT;
      _eos.rho_from_p_T(_u[_qp], _temperature[_qp], rho, drho_dp, drho_dT);
      return drho_dT * _phi[_j][_qp] * vec_vel * _normals[_qp] * _test[_i][_qp];
    }

    default:
      return 0;
  }
}
