#include "OneDEnergyStagnationPandTBC.h"
#include "SinglePhaseFluidProperties.h"
#include "OneDStagnationPandTBase.h"
#include "VolumeFractionMapper.h"
#include "Numerics.h"

registerMooseObject("THMApp", OneDEnergyStagnationPandTBC);

template <>
InputParameters
validParams<OneDEnergyStagnationPandTBC>()
{
  InputParameters params = validParams<OneDNodalBC>();
  params.addParam<bool>(
      "reversible", false, "true, if the boundary condition reversible, otherwise false.");
  params.addRequiredParam<Real>("T0", "Stagnation temperature");
  params.addRequiredParam<Real>("p0", "Stagnation pressure");
  params.addRequiredCoupledVar("A", "Area");
  params.addRequiredCoupledVar("vel", "x-component of the velocity");
  params.addCoupledVar("alpha", 1, "Volume fraction of phase (two-phase only)");
  params.addCoupledVar("beta", 1, "Remapped volume fraction of liquid (two-phase only)");
  params.addRequiredCoupledVar("arhoA", "alpha*rho*A (two-phase) or rho*A (single-phase)");
  params.addRequiredCoupledVar("arhouA", "alpha*rho*u*A (two-phase) or rho*u*A (single-phase)");
  params.addRequiredCoupledVar("arhoEA", "alpha*rho*E*A (two-phase) or rho*E*A (single-phase)");
  params.addParam<bool>("is_liquid", true, "Does the phase correspond to liquid? (two-phase only)");
  params.addRequiredParam<UserObjectName>("fp", "The name of fluid properties user object to use.");
  params.addParam<UserObjectName>("vfm", "The name of the volume fraction mapper user object");

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
    _alpha(coupledValue("alpha")),
    _beta(coupledValue("beta")),
    _arhoA(coupledValue("arhoA")),
    _arhouA(coupledValue("arhouA")),
    _arhoEA(coupledValue("arhoEA")),
    _sign(getParam<bool>("is_liquid") ? 1. : -1.),
    _beta_var_number(isCoupled("beta") ? coupled("beta") : libMesh::invalid_uint),
    _arhoA_var_number(coupled("arhoA")),
    _arhouA_var_number(coupled("arhouA")),
    _vfm(isCoupled("beta") ? &getUserObject<VolumeFractionMapper>("vfm") : nullptr)
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
  return _arhoEA[_qp] - _alpha[_qp] * rho * (e + 0.5 * _vel[_qp] * _vel[_qp]) * _area[_qp];
}

Real
OneDEnergyStagnationPandTBC::computeQpJacobian()
{
  return 1.0;
}

Real
OneDEnergyStagnationPandTBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _beta_var_number)
  {
    // compute density and pressure
    Real rho, p;
    rho_p_from_p0_T0_vel(_p0, _T0, _vel[_qp], rho, p);

    // compute total energy
    const Real e = _fp.e_from_p_rho(p, rho);
    const Real E = e + 0.5 * _vel[_qp] * _vel[_qp];

    return -_sign * (*_vfm).dalpha_liquid_dbeta(_beta[_qp]) * rho * E * _area[_qp];
  }
  else if (jvar == _arhoA_var_number)
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

    return -_alpha[_qp] * _area[_qp] * (E * drho_darhoA + dE_darhoA * rho);
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

    return -_alpha[_qp] * _area[_qp] * (E * drho_darhouA + dE_darhouA * rho);
  }
  else
    return 0;
}
