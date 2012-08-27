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

  // Required parameters
  //params.addRequiredParam<Real>("Hw", "Convective heat transfer coefficient");
  params.addRequiredParam<Real>("aw", "heat transfer area density, m^2 / m^3");  
  params.addRequiredParam<Real>("Tw", "Wall temperature, K");

  params.addRequiredParam<UserObjectName>("eos", "The name of equation of state object to use.");
      
  return params;
}

OneDEnergyWallHeating::OneDEnergyWallHeating(const std::string & name, InputParameters parameters)
    : Kernel(name, parameters),
      _rho(coupledValue("rho")),
      _rhou(coupledValue("rhou")),
      //_u_vel(coupledValue("u")),
      //_pressure(coupledValue("pressure")),
      _temperature(coupledValue("temperature")),
      _rho_var_number(coupled("rho")),  
      //_Hw(getParam<Real>("Hw")), 
      _aw(getParam<Real>("aw")),
      _Tw(getParam<Real>("Tw")),
      _HTC(getMaterialProperty<Real>("HeatTransferCoefficient")),
      _eos(getUserObject<EquationOfState>("eos"))
{}



Real
OneDEnergyWallHeating::computeQpResidual()
{
/*
  //time ramp up
  Real t1 = 0.55;
  Real t2 = 1.0;
  Real _Hw_time = 0.;
  if(_t < t1)
  {
    _Hw_time = 0.;
  }
  else if(_t < t2)
  {
    _Hw_time = _Hw * (_t - t1) / (t2 - t1);
  }
  else
  {
    _Hw_time = _Hw;
  }
*/
  // heat transfer term: Hw * aw * (T-Tw) * psi
  //return _Hw * _aw * (_temperature[_qp]-_Tw) * _test[_i][_qp];
  return _HTC[_qp]* _aw * (_temperature[_qp]-_Tw) * _test[_i][_qp];
}



Real
OneDEnergyWallHeating::computeQpJacobian()
{
  // Derivatives wrt rho*E
  // d(Res)/d(rhoE) = Hw * aw * (dT/drhoE) * phi_j * test
  return _HTC[_qp]* _aw * _eos.dT_drhoE(_rho[_qp], _rhou[_qp], _u[_qp]) * _phi[_j][_qp] * _test[_i][_qp];
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
