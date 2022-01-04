#include "OneDEnergyDensityVelocityBC.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("THMApp", OneDEnergyDensityVelocityBC);

InputParameters
OneDEnergyDensityVelocityBC::validParams()
{
  InputParameters params = OneDIntegratedBC::validParams();
  params.addRequiredParam<Real>("rho", "Density");
  params.addRequiredParam<Real>("vel", "x-component of the velocity");
  params.addRequiredCoupledVar("A", "Area");
  params.addRequiredCoupledVar("rhoEA", "Total energy");
  params.addRequiredParam<UserObjectName>("fp", "The name of fluid properties user object to use.");
  params.declareControllable("rho vel");
  return params;
}

OneDEnergyDensityVelocityBC::OneDEnergyDensityVelocityBC(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<OneDIntegratedBC>(parameters),
    _rho(getParam<Real>("rho")),
    _vel(getParam<Real>("vel")),
    _area(coupledValue("A")),
    _rhoEA(coupledValue("rhoEA")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

Real
OneDEnergyDensityVelocityBC::computeQpResidual()
{
  Real e = _rhoEA[_qp] / _rho / _area[_qp] - 0.5 * _vel * _vel;
  Real p = _fp.p_from_v_e(1 / _rho, e);
  return (_rhoEA[_qp] + p * _area[_qp]) * _vel * _normal * _test[_i][_qp];
}

Real
OneDEnergyDensityVelocityBC::computeQpJacobian()
{
  Real e = _rhoEA[_qp] / _rho / _area[_qp] - 0.5 * _vel * _vel;
  Real p, dp_dv, dp_de;
  _fp.p_from_v_e(1 / _rho, e, p, dp_dv, dp_de);
  Real de_darhoEA = 1.0 / (_rho * _area[_qp]);
  return (1 + _area[_qp] * dp_de * de_darhoEA) * _vel * _normal * _phi[_j][_qp] * _test[_i][_qp];
}

Real
OneDEnergyDensityVelocityBC::computeQpOffDiagJacobian(unsigned int)
{
  return 0;
}
