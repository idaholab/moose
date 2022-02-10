//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OneDEnergyStagnationPandTBC.h"
#include "SinglePhaseFluidProperties.h"
#include "OneDStagnationPandTBase.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", OneDEnergyStagnationPandTBC);

InputParameters
OneDEnergyStagnationPandTBC::validParams()
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

OneDEnergyStagnationPandTBC::OneDEnergyStagnationPandTBC(const InputParameters & parameters)
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
    _arhoA_var_number(coupled("arhoA")),
    _arhouA_var_number(coupled("arhouA"))
{
}

bool
OneDEnergyStagnationPandTBC::shouldApply()
{
  return !_reversible || THM::isInlet(_vel_old[0], _normal);
}

Real
OneDEnergyStagnationPandTBC::computeQpResidual()
{
  // compute density and pressure
  Real rho, p;
  rho_p_from_p0_T0_vel(_p0, _T0, _vel[_qp], rho, p);

  // compute internal energy
  const Real e = _fp.e_from_p_rho(p, rho);

  // See RELAP-7 Theory Manual, pg. 111, Equation (399) {eq:p0_T0_inlet_energy_residual}
  return _arhoEA[_qp] - rho * (e + 0.5 * _vel[_qp] * _vel[_qp]) * _area[_qp];
}

Real
OneDEnergyStagnationPandTBC::computeQpJacobian()
{
  return 1.0;
}

Real
OneDEnergyStagnationPandTBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _arhoA_var_number)
  {
    // compute density and pressure
    Real rho, p;
    rho_p_from_p0_T0_vel(_p0, _T0, _vel[_qp], rho, p);

    // compute total energy
    const Real e = _fp.e_from_p_rho(p, rho);
    const Real E = e + 0.5 * _vel[_qp] * _vel[_qp];

    const Real drho_du = drhodu_from_p0_T0_vel(_p0, _T0, _vel[_qp]);
    const Real du_darhoA = -_arhouA[_qp] / (_arhoA[_qp] * _arhoA[_qp]);
    const Real drho_darhoA = drho_du * du_darhoA;

    const Real dE_du = dEdu_from_p0_T0_vel(_p0, _T0, _vel[_qp]);
    const Real dE_darhoA = dE_du * du_darhoA;

    return -_area[_qp] * (E * drho_darhoA + dE_darhoA * rho);
  }
  else if (jvar == _arhouA_var_number)
  {
    // compute density and pressure
    Real rho, p;
    rho_p_from_p0_T0_vel(_p0, _T0, _vel[_qp], rho, p);

    // compute total energy
    const Real e = _fp.e_from_p_rho(p, rho);
    const Real E = e + 0.5 * _vel[_qp] * _vel[_qp];

    const Real drho_du = drhodu_from_p0_T0_vel(_p0, _T0, _vel[_qp]);
    const Real du_darhouA = 1.0 / _arhoA[_qp];
    const Real drho_darhouA = drho_du * du_darhouA;

    const Real dE_du = dEdu_from_p0_T0_vel(_p0, _T0, _vel[_qp]);
    const Real dE_darhouA = dE_du * du_darhouA;

    return -_area[_qp] * (E * drho_darhouA + dE_darhouA * rho);
  }
  else
    return 0;
}
