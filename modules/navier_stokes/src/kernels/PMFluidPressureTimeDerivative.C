//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PMFluidPressureTimeDerivative.h"

registerMooseObject("NavierStokesApp", PMFluidPressureTimeDerivative);

InputParameters
PMFluidPressureTimeDerivative::validParams()
{
  InputParameters params = FluidPressureTimeDerivative::validParams();
  params.addRequiredCoupledVar("porosity", "porosity");

  return params;
}

PMFluidPressureTimeDerivative::PMFluidPressureTimeDerivative(const InputParameters & parameters)
  : FluidPressureTimeDerivative(parameters), _porosity(coupledValue("porosity"))
{
}

Real
PMFluidPressureTimeDerivative::computeQpResidual()
{
  Real rho, drho_dp, drho_dT;
  _eos.rho_from_p_T(_u[_qp], _temperature[_qp], rho, drho_dp, drho_dT);
  return _porosity[_qp] * (drho_dT * _temperature_dot[_qp] + drho_dp * _u_dot[_qp]) *
         _test[_i][_qp];
}

Real
PMFluidPressureTimeDerivative::computeQpJacobian()
{
  Real rho, drho_dp, drho_dT;
  _eos.rho_from_p_T(_u[_qp], _temperature[_qp], rho, drho_dp, drho_dT);
  return _porosity[_qp] * drho_dp * _du_dot_du[_qp] * _phi[_j][_qp] * _test[_i][_qp];
}

Real
PMFluidPressureTimeDerivative::computeQpOffDiagJacobian(unsigned int jvar)
{
  return _porosity[_qp] * FluidPressureTimeDerivative::computeQpOffDiagJacobian(jvar);
}
