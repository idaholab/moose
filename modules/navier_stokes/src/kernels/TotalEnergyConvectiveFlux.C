//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TotalEnergyConvectiveFlux.h"

registerMooseObject("NavierStokesApp", TotalEnergyConvectiveFlux);

template <>
InputParameters
validParams<TotalEnergyConvectiveFlux>()
{
  InputParameters pars = validParams<Kernel>();

  pars.addRequiredCoupledVar("rho", "density");
  pars.addRequiredCoupledVar("rho_u", "rho*u");
  pars.addCoupledVar("rho_v", "rho*v");
  pars.addCoupledVar("rho_w", "rho*w");
  pars.addRequiredCoupledVar("enthalpy", "Enthalpy");

  return pars;
}

TotalEnergyConvectiveFlux::TotalEnergyConvectiveFlux(const InputParameters & parameters)
  : Kernel(parameters),
    _rho(coupledValue("rho")),
    _rho_u(coupledValue("rho_u")),
    _rho_v(isCoupled("rho_v") ? coupledValue("rho_v") : _zero),
    _rho_w(isCoupled("rho_w") ? coupledValue("rho_w") : _zero),
    _enthalpy(coupledValue("enthalpy"))
{
}

Real
TotalEnergyConvectiveFlux::computeQpResidual()
{
  RealVectorValue rho_u_vec(_rho_u[_qp], _rho_v[_qp], _rho_w[_qp]);
  return -_enthalpy[_qp] * (rho_u_vec * _grad_test[_i][_qp]);
}

Real
TotalEnergyConvectiveFlux::computeQpJacobian()
{
  return 0.;
}
