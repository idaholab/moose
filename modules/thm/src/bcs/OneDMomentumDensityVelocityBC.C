#include "OneDMomentumDensityVelocityBC.h"
#include "SinglePhaseFluidProperties.h"
#include "Numerics.h"

registerMooseObject("THMApp", OneDMomentumDensityVelocityBC);

template <>
InputParameters
validParams<OneDMomentumDensityVelocityBC>()
{
  InputParameters params = validParams<OneDIntegratedBC>();
  params.addRequiredParam<Real>("rho", "The specified density value.");
  params.addRequiredParam<Real>("vel", "The velocity value given as a function.");
  params.addRequiredParam<MaterialPropertyName>("alpha", "Volume fraction");
  params.addRequiredCoupledVar("rhoEA", "Conserved total energy");
  params.addRequiredCoupledVar("A", "Area");
  params.addCoupledVar("beta", "Remapped volume fraction of liquid (two-phase only)");
  params.addRequiredParam<UserObjectName>("fp", "The name of fluid properties user object to use.");
  params.declareControllable("rho vel");
  return params;
}

OneDMomentumDensityVelocityBC::OneDMomentumDensityVelocityBC(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<OneDIntegratedBC>(parameters),
    _rho(getParam<Real>("rho")),
    _vel(getParam<Real>("vel")),
    _alpha(getMaterialProperty<Real>("alpha")),
    _dalpha_dbeta(isCoupled("beta") ? &getMaterialPropertyDerivativeTHM<Real>("alpha", "beta")
                                    : nullptr),
    _area(coupledValue("A")),
    _rhoEA(coupledValue("rhoEA")),
    _rhoEA_var_num(coupled("rhoEA")),
    _beta_var_num(isCoupled("beta") ? coupled("beta") : libMesh::invalid_uint),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

Real
OneDMomentumDensityVelocityBC::computeQpResidual()
{
  Real rhou = _rho * _vel;
  Real e = _rhoEA[_qp] / _alpha[_qp] / _rho / _area[_qp] - 0.5 * _vel * _vel;
  Real p = _fp.p_from_v_e(1 / _rho, e);
  return _alpha[_qp] * (rhou * rhou / _rho + p) * _area[_qp] * _normal * _test[_i][_qp];
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
    Real e = _rhoEA[_qp] / _alpha[_qp] / _rho / _area[_qp] - 0.5 * _vel * _vel;
    Real p, dp_dv, dp_de;
    _fp.p_from_v_e(1 / _rho, e, p, dp_dv, dp_de);
    return dp_de * THM::de_darhoEA(_rho) * _normal * _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _beta_var_num)
  {
    Real rhou = _rho * _vel;
    Real e = _rhoEA[_qp] / _alpha[_qp] / _rho / _area[_qp] - 0.5 * _vel * _vel;
    Real p, dp_dv, dp_de;
    _fp.p_from_v_e(1 / _rho, e, p, dp_dv, dp_de);
    Real de_daL = -_rhoEA[_qp] / _rho / _area[_qp] / _alpha[_qp] / _alpha[_qp];
    return ((rhou * rhou / _rho + p) + _alpha[_qp] * dp_de * de_daL) * (*_dalpha_dbeta)[_qp] *
           _area[_qp] * _normal * _phi[_j][_qp] * _test[_i][_qp];
  }
  else
    return 0;
}
