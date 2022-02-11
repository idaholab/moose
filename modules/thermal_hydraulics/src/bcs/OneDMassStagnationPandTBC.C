//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OneDMassStagnationPandTBC.h"
#include "SinglePhaseFluidProperties.h"
#include "OneDStagnationPandTBase.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", OneDMassStagnationPandTBC);

InputParameters
OneDMassStagnationPandTBC::validParams()
{
  InputParameters params = OneDNodalBC::validParams();
  params.addParam<bool>(
      "reversible", false, "true, if the boundary condition reversible, otherwise false.");
  params.addRequiredParam<Real>("T0", "Stagnation temperature");
  params.addRequiredParam<Real>("p0", "Stagnation pressure");
  params.addRequiredCoupledVar("A", "Area");
  params.addRequiredCoupledVar("vel", "x-component of the velocity");
  params.addRequiredCoupledVar("arhoA", "alpha*rho*A (two-phase) or rho*A (single-phase)");
  params.addRequiredCoupledVar("arhouA", "alpha*rho*u*A (two-phase) or rho*u*A (single-phase)");
  params.addRequiredCoupledVar("arhoEA", "alpha*rho*E*A (two-phase) or rho*E*A (single-phase)");
  params.addRequiredParam<UserObjectName>("fp", "The name of fluid properties user object to use.");

  params.declareControllable("p0 T0");

  return params;
}

OneDMassStagnationPandTBC::OneDMassStagnationPandTBC(const InputParameters & parameters)
  : OneDNodalBC(parameters),
    OneDStagnationPandTBase(getUserObject<SinglePhaseFluidProperties>("fp")),
    _reversible(getParam<bool>("reversible")),
    _T0(getParam<Real>("T0")),
    _p0(getParam<Real>("p0")),
    _area(coupledValue("A")),
    _vel(coupledValue("vel")),
    _vel_old(coupledValueOld("vel")),
    _arhoA(coupledValue("arhoA")),
    _arhouA(coupledValue("arhouA")),
    _arhoEA(coupledValue("arhoEA")),
    _arhouA_var_number(coupled("arhouA")),
    _arhoEA_var_number(coupled("arhoEA"))
{
}

bool
OneDMassStagnationPandTBC::shouldApply()
{
  return !_reversible || THM::isInlet(_vel_old[0], _normal);
}

Real
OneDMassStagnationPandTBC::computeQpResidual()
{
  Real rho, p;
  rho_p_from_p0_T0_vel(_p0, _T0, _vel[_qp], rho, p);

  // See RELAP-7 Theory Manual, pg. 111, Equation (398) {eq:p0_T0_inlet_mass_residual}
  return _arhoA[_qp] - rho * _area[_qp];
}

Real
OneDMassStagnationPandTBC::computeQpJacobian()
{
  const Real drho_du = drhodu_from_p0_T0_vel(_p0, _T0, _vel[_qp]);
  const Real du_darhoA = -_arhouA[_qp] / (_arhoA[_qp] * _arhoA[_qp]);

  const Real drho_darhoA = drho_du * du_darhoA;

  return 1 - _area[_qp] * drho_darhoA;
}

Real
OneDMassStagnationPandTBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _arhouA_var_number)
  {
    const Real drho_du = drhodu_from_p0_T0_vel(_p0, _T0, _vel[_qp]);
    const Real drho_darhouA = drho_du / _arhoA[_qp];
    return -_area[_qp] * drho_darhouA;
  }
  else if (jvar == _arhoEA_var_number)
  {
    return 0;
  }
  else
    return 0;
}
