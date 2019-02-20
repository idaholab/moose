#include "OneDMomentumStagnationPandTBC.h"
#include "SinglePhaseFluidProperties.h"
#include "Numerics.h"

registerMooseObject("THMApp", OneDMomentumStagnationPandTBC);

template <>
InputParameters
validParams<OneDMomentumStagnationPandTBC>()
{
  InputParameters params = validParams<OneDIntegratedBC>();
  params.addParam<bool>(
      "reversible", false, "true, if the boundary condition reversible, otherwise false.");
  params.addRequiredParam<Real>("T0", "Stagnation temperature");
  params.addRequiredParam<Real>("p0", "Stagnation pressure");
  params.addRequiredCoupledVar("A", "Area");
  params.addRequiredCoupledVar("vel", "x-component of the velocity");
  params.addRequiredParam<MaterialPropertyName>("alpha", "Volume fraction");
  params.addCoupledVar("beta", "Remapped volume fraction of liquid (two-phase only)");
  params.addRequiredCoupledVar("arhoA", "alpha*rho*A (two-phase) or rho*A (single-phase)");
  params.addRequiredCoupledVar("arhouA", "alpha*rho*u*A (two-phase) or rho*u*A (single-phase)");
  params.addRequiredCoupledVar("arhoEA", "alpha*rho*E*A (two-phase) or rho*E*A (single-phase)");
  params.addRequiredParam<UserObjectName>("fp", "The name of fluid properties user object to use.");

  params.declareControllable("p0 T0");

  return params;
}

OneDMomentumStagnationPandTBC::OneDMomentumStagnationPandTBC(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<OneDIntegratedBC>(parameters),
    OneDStagnationPandTBase(getUserObject<SinglePhaseFluidProperties>("fp")),
    _reversible(getParam<bool>("reversible")),
    _T0(getParam<Real>("T0")),
    _p0(getParam<Real>("p0")),
    _area(coupledValue("A")),
    _vel(coupledValue("vel")),
    _vel_old(coupledValueOld("vel")),
    _alpha(getMaterialProperty<Real>("alpha")),
    _dalpha_dbeta(isCoupled("beta") ? &getMaterialPropertyDerivativeTHM<Real>("alpha", "beta")
                                    : nullptr),
    _arhoA(coupledValue("arhoA")),
    _arhouA(coupledValue("arhouA")),
    _arhoEA(coupledValue("arhoEA")),
    _beta_var_number(isCoupled("beta") ? coupled("beta") : libMesh::invalid_uint),
    _arhoA_var_number(coupled("arhoA")),
    _arhoEA_var_number(coupled("arhoEA"))
{
}

bool
OneDMomentumStagnationPandTBC::shouldApply()
{
  return !_reversible || THM::isInlet(_vel_old[0], _normal);
}

Real
OneDMomentumStagnationPandTBC::computeQpResidual()
{
  Real rho, p;
  rho_p_from_p0_T0_vel(_p0, _T0, _vel[_qp], rho, p);

  // See RELAP-7 Theory Manual, pg. 111, Equation (400) {eq:p0_T0_inlet_momentum_residual}
  return _alpha[_qp] * _area[_qp] * (rho * _vel[_qp] * _vel[_qp] + p) * _normal * _test[_i][_qp];
}

Real
OneDMomentumStagnationPandTBC::computeQpJacobian()
{
  Real rho, p;
  rho_p_from_p0_T0_vel(_p0, _T0, _vel[_qp], rho, p);

  const Real drho_du = drhodu_from_p0_T0_vel(_p0, _T0, _vel[_qp]);
  const Real du_darhouA = 1.0 / _arhoA[_qp];
  const Real dp_du = dpdu_from_p0_T0_vel(_p0, _T0, _vel[_qp]);

  const Real drho_darhouA = drho_du * du_darhouA;
  const Real du2_darhouA = 2.0 * _vel[_qp] * du_darhouA;
  const Real dp_darhouA = dp_du * du_darhouA;

  return _alpha[_qp] * _area[_qp] *
         (drho_darhouA * _vel[_qp] * _vel[_qp] + rho * du2_darhouA + dp_darhouA) * _phi[_j][_qp] *
         _normal * _test[_i][_qp];
}

Real
OneDMomentumStagnationPandTBC::computeQpOffDiagJacobian(unsigned jvar)
{
  if (jvar == _beta_var_number)
  {
    Real rho, p;
    rho_p_from_p0_T0_vel(_p0, _T0, _vel[_qp], rho, p);

    return (*_dalpha_dbeta)[_qp] * _area[_qp] * (rho * _vel[_qp] * _vel[_qp] + p) * _phi[_j][_qp] *
           _normal * _test[_i][_qp];
  }
  else if (jvar == _arhoA_var_number)
  {
    Real rho, p;
    rho_p_from_p0_T0_vel(_p0, _T0, _vel[_qp], rho, p);

    const Real drho_du = drhodu_from_p0_T0_vel(_p0, _T0, _vel[_qp]);
    const Real du_darhoA = -_arhouA[_qp] / (_arhoA[_qp] * _arhoA[_qp]);
    const Real dp_du = dpdu_from_p0_T0_vel(_p0, _T0, _vel[_qp]);

    const Real drho_darhoA = drho_du * du_darhoA;
    const Real du2_darhoA = 2.0 * _vel[_qp] * du_darhoA;
    const Real dp_darhoA = dp_du * du_darhoA;

    return _alpha[_qp] * _area[_qp] *
           (drho_darhoA * _vel[_qp] * _vel[_qp] + rho * du2_darhoA + dp_darhoA) * _phi[_j][_qp] *
           _normal * _test[_i][_qp];
  }
  else if (jvar == _arhoEA_var_number)
  {
    return 0;
  }
  else
    return 0;
}
