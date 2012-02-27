#include "OneDEnergyWallHeating.h"

template<>
InputParameters validParams<OneDEnergyWallHeating>()
{
  InputParameters params = validParams<Kernel>();
 
  params.addRequiredCoupledVar("rho", ""); // _rho_var_number
  // params.addRequiredCoupledVar("u", "");
  // params.addRequiredCoupledVar("pressure", "");
  params.addRequiredCoupledVar("temperature", "");

  // Required parameters
  params.addRequiredParam<Real>("Hw", "Convective heat transfer coefficient");
  params.addRequiredParam<Real>("aw", "heat transfer area density, m^2 / m^3");  
  params.addRequiredParam<Real>("Tw", "Wall temperature, K");  
      
  return params;
}

OneDEnergyWallHeating::OneDEnergyWallHeating(const std::string & name, InputParameters parameters)
    : Kernel(name, parameters),
      //_rho(coupledValue("rho")),
      //_u_vel(coupledValue("u")),
      //_pressure(coupledValue("pressure")),
      _temperature(coupledValue("temperature")),
      _rho_var_number(coupled("rho")),  
      _Hw(getParam<Real>("Hw")), 
      _aw(getParam<Real>("aw")),
      _Tw(getParam<Real>("Tw"))
{}



Real
OneDEnergyWallHeating::computeQpResidual()
{
  // heat transfer term: Hw * aw * (T-Tw) * psi
  return _Hw * _aw * (_temperature[_qp]-_Tw) * _test[_i][_qp];
}



Real
OneDEnergyWallHeating::computeQpJacobian()
{
  // Derivatives wrt rho*E
  // TODO: Add dT_drho, dT_drhou, dT_drhoE to EOS object
  return 0.;
}



Real
OneDEnergyWallHeating::computeQpOffDiagJacobian(unsigned int jvar)
{

  if (jvar == _rho_var_number) 
  {
  // Derivatives wrt rho*E
  // TODO: Add dT_drho, dT_drhou, dT_drhoE to EOS object
  return 0.;
  }
  else
    return 0.;
}
