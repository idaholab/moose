#include "OneDEnergyWallHeating.h"
#include "SinglePhaseCommonFluidProperties.h"

template<>
InputParameters validParams<OneDEnergyWallHeating>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("rhoA", "");
  params.addRequiredCoupledVar("rhouA", "");
  params.addRequiredCoupledVar("rho", "density");
  params.addRequiredCoupledVar("rhou", "momentum");
  params.addRequiredCoupledVar("rhoE", "total energy");
  params.addRequiredCoupledVar("temperature", "Fluid temperature");
  params.addRequiredCoupledVar("heat_transfer_coefficient", "convective heat transfer coefficient, W/m^2-K");
  params.addRequiredCoupledVar("heat_flux_perimeter", "heat flux perimeter");
  params.addCoupledVar("Tw", 0, "Wall temperature (const)");
  params.addRequiredCoupledVar("area", "area of the pipe, coupled as an aux variable");
  params.addRequiredParam<UserObjectName>("fp", "The name of fluid properties user object to use.");
  return params;
}

OneDEnergyWallHeating::OneDEnergyWallHeating(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _rho(coupledValue("rho")),
    _rhou(coupledValue("rhou")),
    _rhoE(coupledValue("rhoE")),
    _temperature(coupledValue("temperature")),
    _heat_transfer_coefficient(coupledValue("heat_transfer_coefficient")),
    _Tw(coupledValue("Tw")),
    _area(coupledValue("area")),
    _rhoA_var_number(coupled("rhoA")),
    _rhouA_var_number(coupled("rhouA")),
    _Phf(coupledValue("heat_flux_perimeter")),
    _spfp(getUserObject<SinglePhaseCommonFluidProperties>("fp"))
{
}

OneDEnergyWallHeating::~OneDEnergyWallHeating()
{
}


Real
OneDEnergyWallHeating::computeQpResidual()
{
  return _heat_transfer_coefficient[_qp] * _Phf[_qp] * (_temperature[_qp] - _Tw[_qp]) * _test[_i][_qp];
}


Real
OneDEnergyWallHeating::computeQpJacobian()
{
  // Derivatives wrt rho*E.
  return _heat_transfer_coefficient[_qp] * _Phf[_qp] * (_spfp.dT_drhoE(_rho[_qp], _rhou[_qp], _rhoE[_qp]) / _area[_qp]) * _phi[_j][_qp] * _test[_i][_qp];
}


Real
OneDEnergyWallHeating::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _rhoA_var_number)  // Derivative wrt rho
    return _heat_transfer_coefficient[_qp] * _Phf[_qp] * (_spfp.dT_drho(_rho[_qp], _rhou[_qp], _rhoE[_qp]) / _area[_qp]) * _phi[_j][_qp] * _test[_i][_qp];

  else if (jvar == _rhouA_var_number)  // Derivative wrt rhou
    return _heat_transfer_coefficient[_qp] * _Phf[_qp] * (_spfp.dT_drhou(_rho[_qp], _rhou[_qp], _rhoE[_qp]) / _area[_qp]) * _phi[_j][_qp] * _test[_i][_qp];

  else
    return 0.;
}
