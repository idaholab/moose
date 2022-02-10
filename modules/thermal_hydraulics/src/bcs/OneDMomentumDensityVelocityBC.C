//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OneDMomentumDensityVelocityBC.h"
#include "SinglePhaseFluidProperties.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", OneDMomentumDensityVelocityBC);

InputParameters
OneDMomentumDensityVelocityBC::validParams()
{
  InputParameters params = OneDIntegratedBC::validParams();
  params.addRequiredParam<Real>("rho", "The specified density value.");
  params.addRequiredParam<Real>("vel", "The velocity value given as a function.");
  params.addRequiredCoupledVar("rhoEA", "Conserved total energy");
  params.addRequiredCoupledVar("A", "Area");
  params.addRequiredParam<UserObjectName>("fp", "The name of fluid properties user object to use.");
  params.declareControllable("rho vel");
  return params;
}

OneDMomentumDensityVelocityBC::OneDMomentumDensityVelocityBC(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<OneDIntegratedBC>(parameters),
    _rho(getParam<Real>("rho")),
    _vel(getParam<Real>("vel")),
    _area(coupledValue("A")),
    _rhoEA(coupledValue("rhoEA")),
    _rhoEA_var_num(coupled("rhoEA")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

Real
OneDMomentumDensityVelocityBC::computeQpResidual()
{
  Real rhou = _rho * _vel;
  Real e = _rhoEA[_qp] / _rho / _area[_qp] - 0.5 * _vel * _vel;
  Real p = _fp.p_from_v_e(1 / _rho, e);
  return (rhou * rhou / _rho + p) * _area[_qp] * _normal * _test[_i][_qp];
}

Real
OneDMomentumDensityVelocityBC::computeQpJacobian()
{
  return 0;
}

Real
OneDMomentumDensityVelocityBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _rhoEA_var_num)
  {
    Real e = _rhoEA[_qp] / _rho / _area[_qp] - 0.5 * _vel * _vel;
    Real p, dp_dv, dp_de;
    _fp.p_from_v_e(1 / _rho, e, p, dp_dv, dp_de);
    return dp_de * THM::de_darhoEA(_rho) * _normal * _phi[_j][_qp] * _test[_i][_qp];
  }
  else
    return 0;
}
