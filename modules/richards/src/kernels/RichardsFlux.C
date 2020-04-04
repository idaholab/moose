//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RichardsFlux.h"

// MOOSE includes
#include "Material.h"
#include "MooseVariable.h"

// C++ includes
#include <iostream>

registerMooseObject("RichardsApp", RichardsFlux);

InputParameters
RichardsFlux::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addParam<bool>(
      "linear_shape_fcns",
      true,
      "If you are using second-order Lagrange shape functions you need to set this to false.");
  params.addRequiredParam<UserObjectName>(
      "richardsVarNames_UO", "The UserObject that holds the list of Richards variable names.");
  return params;
}

RichardsFlux::RichardsFlux(const InputParameters & parameters)
  : Kernel(parameters),
    _richards_name_UO(getUserObject<RichardsVarNames>("richardsVarNames_UO")),
    _pvar(_richards_name_UO.richards_var_num(_var.number())),

    // This kernel gets lots of things from the material
    _flux(getMaterialProperty<std::vector<RealVectorValue>>("flux")),
    _dflux_dv(getMaterialProperty<std::vector<std::vector<RealVectorValue>>>("dflux_dv")),
    _dflux_dgradv(getMaterialProperty<std::vector<std::vector<RealTensorValue>>>("dflux_dgradv")),
    _d2flux_dvdv(
        getMaterialProperty<std::vector<std::vector<std::vector<RealVectorValue>>>>("d2flux_dvdv")),
    _d2flux_dgradvdv(getMaterialProperty<std::vector<std::vector<std::vector<RealTensorValue>>>>(
        "d2flux_dgradvdv")),
    _d2flux_dvdgradv(getMaterialProperty<std::vector<std::vector<std::vector<RealTensorValue>>>>(
        "d2flux_dvdgradv")),

    _second_u(getParam<bool>("linear_shape_fcns")
                  ? _second_zero
                  : (_is_implicit ? _var.secondSln() : _var.secondSlnOld())),
    _second_phi(getParam<bool>("linear_shape_fcns") ? _second_phi_zero : secondPhi()),

    _tauvel_SUPG(getMaterialProperty<std::vector<RealVectorValue>>("tauvel_SUPG")),
    _dtauvel_SUPG_dgradv(
        getMaterialProperty<std::vector<std::vector<RealTensorValue>>>("dtauvel_SUPG_dgradv")),
    _dtauvel_SUPG_dv(
        getMaterialProperty<std::vector<std::vector<RealVectorValue>>>("dtauvel_SUPG_dv"))
{
}

Real
RichardsFlux::computeQpResidual()
{
  Real flux_part = _grad_test[_i][_qp] * _flux[_qp][_pvar];

  Real supg_test = _tauvel_SUPG[_qp][_pvar] * _grad_test[_i][_qp];
  Real supg_kernel = 0.0;
  if (supg_test != 0)
    // NOTE: Libmesh does not correctly calculate grad(_grad_u) correctly, so following might not be
    // correct
    // NOTE: The following is -divergence(flux)
    // NOTE: The following must be generalised if a non PPPP formalism is used
    // NOTE: The generalisation will look like
    // supg_kernel = sum_j {-(_dflux_dgradv[_qp][_pvar][j]*_second_u[_qp][j]).tr() -
    // _dflux_dv[_qp][_pvar][j]*_grad_u[_qp][j]}
    // where _grad_u[_qp][j] is the gradient of the j^th variable at the quadpoint.
    supg_kernel = -(_dflux_dgradv[_qp][_pvar][_pvar] * _second_u[_qp]).tr() -
                  _dflux_dv[_qp][_pvar][_pvar] * _grad_u[_qp];

  return flux_part + supg_test * supg_kernel;
}

Real
RichardsFlux::computeQpJac(unsigned int wrt_num)
{
  Real flux_prime = _grad_test[_i][_qp] * (_dflux_dgradv[_qp][_pvar][wrt_num] * _grad_phi[_j][_qp] +
                                           _dflux_dv[_qp][_pvar][wrt_num] * _phi[_j][_qp]);

  Real supg_test = _tauvel_SUPG[_qp][_pvar] * _grad_test[_i][_qp];
  Real supg_test_prime =
      _grad_phi[_j][_qp] * (_dtauvel_SUPG_dgradv[_qp][_pvar][wrt_num] * _grad_test[_i][_qp]) +
      _phi[_j][_qp] * _dtauvel_SUPG_dv[_qp][_pvar][wrt_num] * _grad_test[_i][_qp];
  Real supg_kernel = 0.0;
  Real supg_kernel_prime = 0.0;

  if (supg_test != 0)
  {
    // NOTE: since Libmesh does not correctly calculate grad(_grad_u) correctly, so following might
    // not be correct
    supg_kernel = -(_dflux_dgradv[_qp][_pvar][_pvar] * _second_u[_qp]).tr() -
                  _dflux_dv[_qp][_pvar][_pvar] * _grad_u[_qp];

    // NOTE: just like supg_kernel, this must be generalised for non-PPPP formulations
    supg_kernel_prime =
        -(_d2flux_dvdv[_qp][_pvar][_pvar][wrt_num] * _phi[_j][_qp] * _grad_u[_qp] +
          _phi[_j][_qp] * (_d2flux_dgradvdv[_qp][_pvar][_pvar][wrt_num] * _second_u[_qp]).tr() +
          (_d2flux_dvdgradv[_qp][_pvar][_pvar][wrt_num] * _grad_u[_qp]) * _grad_phi[_j][_qp]);
    if (wrt_num == _pvar)
      supg_kernel_prime -= _dflux_dv[_qp][_pvar][_pvar] * _grad_phi[_j][_qp];
    supg_kernel_prime -= (_dflux_dgradv[_qp][_pvar][_pvar] * _second_phi[_j][_qp]).tr();
  }

  return flux_prime + supg_test_prime * supg_kernel + supg_test * supg_kernel_prime;
}

Real
RichardsFlux::computeQpJacobian()
{
  return computeQpJac(_pvar);
}

Real
RichardsFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (_richards_name_UO.not_richards_var(jvar))
    return 0.0;
  unsigned int dvar = _richards_name_UO.richards_var_num(jvar);
  return computeQpJac(dvar);
}
