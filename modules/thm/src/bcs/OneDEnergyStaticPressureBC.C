#include "OneDEnergyStaticPressureBC.h"
#include "SinglePhaseFluidProperties.h"
#include "Numerics.h"

registerMooseObject("THMApp", OneDEnergyStaticPressureBC);

InputParameters
OneDEnergyStaticPressureBC::validParams()
{
  InputParameters params = OneDNodalBC::validParams();
  params.addParam<bool>("reversible", false, "true for reversible behavior");
  params.addRequiredCoupledVar("A", "Area");
  params.addRequiredCoupledVar("rho", "Density of the phase");
  params.addRequiredCoupledVar("rhoA", "Conserved density of the phase");
  params.addRequiredCoupledVar("rhouA", "Conserved momentum of the phase");
  params.addRequiredCoupledVar("vel", "x-component of phase velocity");
  params.addRequiredCoupledVar("v", "Specific volume of the phase");
  params.addRequiredCoupledVar("e", "Specific internal energy of the phase");
  params.addRequiredParam<Real>("p_in", "The desired static pressure at the boundary");
  params.addRequiredParam<UserObjectName>("fp", "The name of the fluid property user object");

  params.declareControllable("p_in");

  return params;
}

OneDEnergyStaticPressureBC::OneDEnergyStaticPressureBC(const InputParameters & parameters)
  : OneDNodalBC(parameters),
    _reversible(getParam<bool>("reversible")),
    _area(coupledValue("A")),
    _rho(coupledValue("rho")),
    _rhoA(coupledValue("rhoA")),
    _rhouA(coupledValue("rhouA")),
    _vel(coupledValue("vel")),
    _vel_old(coupledValueOld("vel")),
    _v_old(coupledValueOld("v")),
    _e_old(coupledValueOld("e")),
    _rhoA_var_number(coupled("rhoA")),
    _rhouA_var_number(coupled("rhouA")),
    _p_in(getParam<Real>("p_in")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

bool
OneDEnergyStaticPressureBC::shouldApply()
{
  if (!_reversible || THM::isOutlet(_vel_old[0], _normal))
  {
    // use old value to prevent BC from oscillating around M = 1 and failing to solve
    Real M = _vel_old[0] / _fp.c_from_v_e(_v_old[0], _e_old[0]);
    return (M <= 1);
  }
  else
    return false;
}

Real
OneDEnergyStaticPressureBC::computeQpResidual()
{
  Real E = _fp.e_from_p_rho(_p_in, _rho[_qp]) + 0.5 * _vel[_qp] * _vel[_qp];
  Real arhoEA = _rhoA[_qp] * E;
  return _u[_qp] - arhoEA;
}

Real
OneDEnergyStaticPressureBC::computeQpJacobian()
{
  return 1;
}

Real
OneDEnergyStaticPressureBC::computeQpOffDiagJacobian(unsigned jvar)
{
  if (jvar == _rhoA_var_number)
  {
    Real drho_darhoA = 1 / _area[_qp];

    Real e, de_dp, de_drho;
    _fp.e_from_p_rho(_p_in, _rho[_qp], e, de_dp, de_drho);
    Real E = e + 0.5 * _vel[_qp] * _vel[_qp];
    Real dE_drhoA = de_drho * drho_darhoA - _vel[_qp] * _vel[_qp] / _rhoA[_qp];

    return -(E + _rhoA[_qp] * dE_drhoA);
  }
  else if (jvar == _rhouA_var_number)
  {
    Real dE_drhouA = _vel[_qp] / _rhoA[_qp];

    return -_rhoA[_qp] * dE_drhouA;
  }
  else
    return 0.;
}
