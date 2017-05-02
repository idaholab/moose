#include "OneDEnergyWallHeating.h"

template <>
InputParameters
validParams<OneDEnergyWallHeating>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("rhoA", "");
  params.addRequiredCoupledVar("rhouA", "");
  params.addRequiredCoupledVar("rhoEA", "Energy equation variable");
  params.addRequiredCoupledVar("heat_transfer_coefficient",
                               "convective heat transfer coefficient, W/m^2-K");
  params.addRequiredCoupledVar("heat_flux_perimeter", "heat flux perimeter");
  params.addCoupledVar("T_wall", 0, "Wall temperature (const)");
  return params;
}

OneDEnergyWallHeating::OneDEnergyWallHeating(const InputParameters & parameters)
  : DerivativeMaterialInterfaceRelap<Kernel>(parameters),
    _temperature(getMaterialPropertyByName<Real>("temperature")),
    _dT_drhoA(getMaterialPropertyDerivativeRelap<Real>("temperature", "rhoA")),
    _dT_drhouA(getMaterialPropertyDerivativeRelap<Real>("temperature", "rhouA")),
    _dT_drhoEA(getMaterialPropertyDerivativeRelap<Real>("temperature", "rhoEA")),
    _heat_transfer_coefficient(coupledValue("heat_transfer_coefficient")),
    _T_wall(coupledValue("T_wall")),
    _Phf(coupledValue("heat_flux_perimeter")),
    _rhoA_var_number(coupled("rhoA")),
    _rhouA_var_number(coupled("rhouA"))
{
}

OneDEnergyWallHeating::~OneDEnergyWallHeating() {}

Real
OneDEnergyWallHeating::computeQpResidual()
{
  return _heat_transfer_coefficient[_qp] * _Phf[_qp] * (_temperature[_qp] - _T_wall[_qp]) *
         _test[_i][_qp];
}

Real
OneDEnergyWallHeating::computeQpJacobian()
{
  return _heat_transfer_coefficient[_qp] * _Phf[_qp] * _dT_drhoEA[_qp] * _phi[_j][_qp] *
         _test[_i][_qp];
}

Real
OneDEnergyWallHeating::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _rhoA_var_number)
    return _heat_transfer_coefficient[_qp] * _Phf[_qp] * _dT_drhoA[_qp] * _phi[_j][_qp] *
           _test[_i][_qp];

  else if (jvar == _rhouA_var_number)
    return _heat_transfer_coefficient[_qp] * _Phf[_qp] * _dT_drhouA[_qp] * _phi[_j][_qp] *
           _test[_i][_qp];

  else
    return 0.;
}
