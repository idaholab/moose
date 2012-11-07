#include "OneDEnergyWallHeating.h"
#include "EquationOfState.h"

template<>
InputParameters validParams<OneDEnergyWallHeating>()
{
  InputParameters params = validParams<Kernel>();
 
  params.addRequiredCoupledVar("rho", "density"); // _rho_var_number
  params.addRequiredCoupledVar("rhou", "momentum"); // _rho_var_number
  // params.addRequiredCoupledVar("u", "");
  // params.addRequiredCoupledVar("pressure", "");
  params.addRequiredCoupledVar("temperature", "");
  params.addRequiredCoupledVar("heat_transfer_coefficient", ""); 

  // Required parameters
  params.addRequiredParam<Real>("aw", "heat transfer area density, m^2 / m^3");  
  params.addRequiredParam<Real>("Tw", "Wall temperature, K");

  params.addRequiredParam<UserObjectName>("eos", "The name of equation of state object to use.");
      
  return params;
}

OneDEnergyWallHeating::OneDEnergyWallHeating(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _rho(coupledValue("rho")),
    _rhou(coupledValue("rhou")),
    _temperature(coupledValue("temperature")),
    _HTC_aux(coupledValue("heat_transfer_coefficient")),
    _rho_var_number(coupled("rho")),
    _rhou_var_number(coupled("rhou")),
    _aw(getParam<Real>("aw")),
    _Tw(getParam<Real>("Tw")),
    _eos(getUserObject<EquationOfState>("eos"))
{}



Real
OneDEnergyWallHeating::computeQpResidual()
{
  // heat transfer term: Hw * aw * (T-Tw) * psi
  return _HTC_aux[_qp] * _aw * (_temperature[_qp]-_Tw) * _test[_i][_qp];
}



Real
OneDEnergyWallHeating::computeQpJacobian()
{
  // Derivatives wrt rho*E
  // d(Res)/d(rhoE) = Hw * aw * (dT/drhoE) * phi_j * test
  return _HTC_aux[_qp] * _aw * _eos.dT_drhoE(_rho[_qp], _rhou[_qp], _u[_qp]) * _phi[_j][_qp] * _test[_i][_qp];  
}



Real
OneDEnergyWallHeating::computeQpOffDiagJacobian(unsigned int jvar)
{
  Real jac = 0.;

  if (jvar == _rho_var_number) 
  {
    jac = _HTC_aux[_qp] * _aw * _eos.dT_drho(_rho[_qp], _rhou[_qp], _u[_qp]) * _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _rhou_var_number) 
  {
    jac = _HTC_aux[_qp] * _aw * _eos.dT_drhou(_rho[_qp], _rhou[_qp], _u[_qp]) * _phi[_j][_qp] * _test[_i][_qp];
  }
  else
    jac = 0.;

  return jac;
}
