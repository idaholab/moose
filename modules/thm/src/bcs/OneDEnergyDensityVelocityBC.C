#include "OneDEnergyDensityVelocityBC.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("THMApp", OneDEnergyDensityVelocityBC);

template <>
InputParameters
validParams<OneDEnergyDensityVelocityBC>()
{
  InputParameters params = validParams<OneDIntegratedBC>();
  params.addRequiredParam<Real>("rho", "Density");
  params.addRequiredParam<Real>("vel", "x-component of the velocity");
  params.addRequiredParam<MaterialPropertyName>("alpha", "Volume fraction");
  params.addRequiredCoupledVar("A", "Area");
  params.addRequiredCoupledVar("rhoEA", "Total energy");
  params.addCoupledVar("beta", "Remapped volume fraction of liquid (two-phase only)");
  params.addRequiredParam<UserObjectName>("fp", "The name of fluid properties user object to use.");
  params.declareControllable("rho vel");
  return params;
}

OneDEnergyDensityVelocityBC::OneDEnergyDensityVelocityBC(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<OneDIntegratedBC>(parameters),
    _rho(getParam<Real>("rho")),
    _vel(getParam<Real>("vel")),
    _alpha(getMaterialProperty<Real>("alpha")),
    _dalpha_dbeta(isCoupled("beta") ? &getMaterialPropertyDerivativeTHM<Real>("alpha", "beta")
                                    : nullptr),
    _area(coupledValue("A")),
    _rhoEA(coupledValue("rhoEA")),
    _beta_var_num(isCoupled("beta") ? coupled("beta") : libMesh::invalid_uint),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

Real
OneDEnergyDensityVelocityBC::computeQpResidual()
{
  Real e = _rhoEA[_qp] / _alpha[_qp] / _rho / _area[_qp] - 0.5 * _vel * _vel;
  Real p = _fp.p_from_v_e(1 / _rho, e);
  return (_rhoEA[_qp] + _alpha[_qp] * p * _area[_qp]) * _vel * _normal * _test[_i][_qp];
}

Real
OneDEnergyDensityVelocityBC::computeQpJacobian()
{
  Real e = _rhoEA[_qp] / _alpha[_qp] / _rho / _area[_qp] - 0.5 * _vel * _vel;
  Real p, dp_dv, dp_de;
  _fp.p_from_v_e(1 / _rho, e, p, dp_dv, dp_de);
  Real de_darhoEA = 1.0 / (_alpha[_qp] * _rho * _area[_qp]);
  return (1 + _alpha[_qp] * _area[_qp] * dp_de * de_darhoEA) * _vel * _normal * _phi[_j][_qp] *
         _test[_i][_qp];
}

Real
OneDEnergyDensityVelocityBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _beta_var_num)
  {
    Real e = _rhoEA[_qp] / _alpha[_qp] / _rho / _area[_qp] - 0.5 * _vel * _vel;
    Real p, dp_dv, dp_de;
    _fp.p_from_v_e(1 / _rho, e, p, dp_dv, dp_de);
    Real de_dalpha = -_rhoEA[_qp] / _rho / _area[_qp] / _alpha[_qp] / _alpha[_qp];
    return (p * _area[_qp] + _alpha[_qp] * _area[_qp] * dp_de * de_dalpha) * (*_dalpha_dbeta)[_qp] *
           _vel * _normal * _phi[_j][_qp] * _test[_i][_qp];
  }
  else
    return 0;
}
