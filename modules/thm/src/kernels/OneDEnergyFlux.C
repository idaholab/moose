#include "OneDEnergyFlux.h"
#include "SinglePhaseCommonFluidProperties.h"

template<>
InputParameters validParams<OneDEnergyFlux>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("area", "Cross-sectional area");
  params.addRequiredCoupledVar("rhoA", "density multiplied by area");
  params.addRequiredCoupledVar("rhouA", "momentum multiplied by area");
  params.addRequiredCoupledVar("u", "velocity");
  params.addRequiredCoupledVar("enthalpy", "enthalpy");
  params.addParam<bool>("is_liquid", true, "True for liquid, false for vapor");
  params.addCoupledVar("alpha", 1., "Volume fraction");
  params.addCoupledVar("alpha_A_liquid", "Volume fraction of liquid");
  params.addRequiredParam<MaterialPropertyName>("pressure", "Pressure");
  params.addRequiredParam<MaterialPropertyName>("dp_drho", "Derivative of pressure w.r.t. density");
  params.addRequiredParam<MaterialPropertyName>("dp_drhou", "Derivative of pressure w.r.t. momentum");
  params.addRequiredParam<MaterialPropertyName>("dp_drhoE", "Derivative of pressure w.r.t. total energy");
  params.addParam<MaterialPropertyName>("dp_dalphaAL", "Derivative of pressure w.r.t. alpha_A_liquid");

  return params;
}

OneDEnergyFlux::OneDEnergyFlux(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _is_liquid(getParam<bool>("is_liquid")),
    _sign(_is_liquid ? 1. : -1.),
    _rhouA(coupledValue("rhouA")),
    _area(coupledValue("area")),
    _u_vel(coupledValue("u")),
    _enthalpy(coupledValue("enthalpy")),
    _pressure(getMaterialProperty<Real>("pressure")),
    _dp_drho(getMaterialProperty<Real>("dp_drho")),
    _dp_drhou(getMaterialProperty<Real>("dp_drhou")),
    _dp_drhoE(getMaterialProperty<Real>("dp_drhoE")),
    _rhoA_var_number(coupled("rhoA")),
    _rhouA_var_number(coupled("rhouA")),
    _has_alpha_A(isCoupled("alpha_A_liquid")),
    _alpha_A_liquid_var_number(_has_alpha_A ? coupled("alpha_A_liquid") : libMesh::invalid_uint),
    _dp_dalphaAL(_has_alpha_A ? &getMaterialProperty<Real>("dp_dalphaAL") : NULL),
    _alpha(coupledValue("alpha"))
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
  Real A33 = _u_vel[_qp] * (1. + _dp_drhoE[_qp]);
  return -A33 * _phi[_j][_qp] * _grad_test[_i][_qp](0);
}

Real
OneDEnergyFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _rhoA_var_number)
  {
    Real A31 = _u_vel[_qp] * (_dp_drho[_qp] - _enthalpy[_qp]);
    return -A31 * _phi[_j][_qp] * _grad_test[_i][_qp](0);
  }
  else if (jvar == _rhouA_var_number)
  {
    Real A32 = _u_vel[_qp] * _dp_drhou[_qp] + _enthalpy[_qp];
    return -A32 * _phi[_j][_qp] * _grad_test[_i][_qp](0);
  }
  else if (jvar == _alpha_A_liquid_var_number)
  {
    return -(_u_vel[_qp] * (_sign * _pressure[_qp] + _alpha[_qp] * _area[_qp] * (*_dp_dalphaAL)[_qp])) * _phi[_j][_qp] * _grad_test[_i][_qp](0);
  }
  else
    return 0.;
}
