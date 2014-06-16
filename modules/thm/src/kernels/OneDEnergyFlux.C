#include "OneDEnergyFlux.h"
#include "EquationOfState.h"

template<>
InputParameters validParams<OneDEnergyFlux>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("rho", "density");
  params.addRequiredCoupledVar("rhou", "momentum");
  params.addRequiredCoupledVar("rhoE", "total energy");

  params.addRequiredCoupledVar("rhoA", "density multiplied by area");
  params.addRequiredCoupledVar("rhouA", "momentum multiplied by area");

  params.addRequiredCoupledVar("u", "velocity");
  params.addRequiredCoupledVar("enthalpy", "enthalpy");

  params.addRequiredParam<UserObjectName>("eos", "The name of equation of state object to use.");

  return params;
}

OneDEnergyFlux::OneDEnergyFlux(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _rhouA(coupledValue("rhouA")),
    _rho(coupledValue("rho")),
    _rhou(coupledValue("rhou")),
    _rhoE(coupledValue("rhoE")),
    _u_vel(coupledValue("u")),
    _enthalpy(coupledValue("enthalpy")),
    _rhoA_var_number(coupled("rhoA")),
    _rhouA_var_number(coupled("rhouA")),
    _eos(getUserObject<EquationOfState>("eos"))
{
}

OneDEnergyFlux::~OneDEnergyFlux()
{
}

Real
OneDEnergyFlux::computeQpResidual()
{
  return -_rhouA[_qp] * _enthalpy[_qp] * _grad_test[_i][_qp](0);
}

Real
OneDEnergyFlux::computeQpJacobian()
{
  Real A33 = _u_vel[_qp] * (1. + _eos.dp_drhoE(_rho[_qp], _rhou[_qp], _rhoE[_qp]));
  return -A33 * _phi[_j][_qp] * _grad_test[_i][_qp](0);
}

Real
OneDEnergyFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _rhoA_var_number)
  {
    Real A31 = _u_vel[_qp] * (_eos.dp_drho(_rho[_qp], _rhou[_qp], _rhoE[_qp]) - _enthalpy[_qp]);
    return -A31 * _phi[_j][_qp] * _grad_test[_i][_qp](0);
  }
  else if (jvar == _rhouA_var_number)
  {
    Real A32 = _u_vel[_qp] * _eos.dp_drhou(_rho[_qp], _rhou[_qp], _rhoE[_qp]) + _enthalpy[_qp];
    return -A32 * _phi[_j][_qp] * _grad_test[_i][_qp](0);
  }
  else
    return 0.;
}
