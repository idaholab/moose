#include "OneDEnergyFlux.h"
#include "EquationOfState.h"

template<>
InputParameters validParams<OneDEnergyFlux>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("area", "Cross-sectional area");
  params.addRequiredCoupledVar("rho", "density");
  params.addRequiredCoupledVar("rhou", "momentum");
  params.addRequiredCoupledVar("rhoE", "total energy");

  params.addRequiredCoupledVar("rhoA", "density multiplied by area");
  params.addRequiredCoupledVar("rhouA", "momentum multiplied by area");

  params.addRequiredCoupledVar("u", "velocity");
  params.addRequiredCoupledVar("enthalpy", "enthalpy");
  params.addRequiredCoupledVar("pressure", "pressure");

  params.addRequiredParam<bool>("is_liquid", "True for liquid, false for vapor");
  params.addCoupledVar("alpha", 1., "Volume fraction");
  params.addCoupledVar("alpha_A_liquid", "Volume fraction of liquid");

  params.addRequiredParam<UserObjectName>("eos", "The name of equation of state object to use.");

  return params;
}

OneDEnergyFlux::OneDEnergyFlux(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _is_liquid(getParam<bool>("is_liquid")),
    _sign(_is_liquid ? 1. : -1.),
    _rhouA(coupledValue("rhouA")),
    _area(coupledValue("area")),
    _rho(coupledValue("rho")),
    _rhou(coupledValue("rhou")),
    _rhoE(coupledValue("rhoE")),
    _u_vel(coupledValue("u")),
    _enthalpy(coupledValue("enthalpy")),
    _pressure(coupledValue("pressure")),
    _rhoA_var_number(coupled("rhoA")),
    _rhouA_var_number(coupled("rhouA")),
    _has_alpha_A(isCoupled("alpha_A_liquid")),
    _alpha_A_liquid_var_number(_has_alpha_A ? coupled("alpha_A_liquid") : libMesh::invalid_uint),
    _dp_dalphaA_liquid(_has_alpha_A ?
        (_is_liquid ? &getMaterialProperty<Real>("dp_L_d_alphaA_L") : &getMaterialProperty<Real>("dp_V_d_alphaA_L")) :
        NULL),
    _alpha(coupledValue("alpha")),
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
  else if (jvar == _alpha_A_liquid_var_number)
  {
    return -(_u_vel[_qp] * (_sign * _pressure[_qp] + _alpha[_qp] * _area[_qp] * (*_dp_dalphaA_liquid)[_qp])) * _phi[_j][_qp] * _grad_test[_i][_qp](0);
  }
  else
    return 0.;
}
