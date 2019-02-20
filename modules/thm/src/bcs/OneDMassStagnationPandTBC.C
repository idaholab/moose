#include "OneDMassStagnationPandTBC.h"
#include "SinglePhaseFluidProperties.h"
#include "OneDStagnationPandTBase.h"
#include "VolumeFractionMapper.h"
#include "Numerics.h"

registerMooseObject("THMApp", OneDMassStagnationPandTBC);

template <>
InputParameters
validParams<OneDMassStagnationPandTBC>()
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

OneDMassStagnationPandTBC::OneDMassStagnationPandTBC(const InputParameters & parameters)
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
    _arhouA_var_number(coupled("arhouA")),
    _arhoEA_var_number(coupled("arhoEA")),
    _vfm(isCoupled("beta") ? &getUserObject<VolumeFractionMapper>("vfm") : nullptr)
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
  return _arhoA[_qp] - _alpha[_qp] * rho * _area[_qp];
}

Real
OneDMassStagnationPandTBC::computeQpJacobian()
{
  const Real drho_du = drhodu_from_p0_T0_vel(_p0, _T0, _vel[_qp]);
  const Real du_darhoA = -_arhouA[_qp] / (_arhoA[_qp] * _arhoA[_qp]);

  const Real drho_darhoA = drho_du * du_darhoA;

  return 1 - _alpha[_qp] * _area[_qp] * drho_darhoA;
}

Real
OneDMassStagnationPandTBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _beta_var_number)
  {
    Real rho, p_dummy;
    rho_p_from_p0_T0_vel(_p0, _T0, _vel[_qp], rho, p_dummy);
    return -_sign * (*_vfm).dalpha_liquid_dbeta(_beta[_qp]) * rho * _area[_qp];
  }
  else if (jvar == _arhouA_var_number)
  {
    const Real drho_du = drhodu_from_p0_T0_vel(_p0, _T0, _vel[_qp]);
    const Real drho_darhouA = drho_du / _arhoA[_qp];
    return -_alpha[_qp] * _area[_qp] * drho_darhouA;
  }
  else if (jvar == _arhoEA_var_number)
  {
    return 0;
  }
  else
    return 0;
}
