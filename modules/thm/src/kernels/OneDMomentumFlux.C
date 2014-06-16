#include "OneDMomentumFlux.h"
#include "EquationOfState.h"

template<>
InputParameters validParams<OneDMomentumFlux>()
{
  InputParameters params = validParams<Kernel>();
  params.addCoupledVar("alpha", 1.0, "Volume fraction");
  params.addCoupledVar("alpha_A_liquid", "Volume fraction of liquid");

  params.addRequiredCoupledVar("rho", "density");
  params.addRequiredCoupledVar("rhou", "momentum");
  params.addCoupledVar("rhoE", "total energy");

  params.addRequiredCoupledVar("rhoA", "density multiplied by area");
  params.addCoupledVar("rhoEA", "total energy multiplied by area");

  params.addRequiredCoupledVar("u", "velocity");
  params.addRequiredCoupledVar("pressure", "pressure");
  params.addRequiredCoupledVar("area", "cross-sectional area");

  params.addRequiredParam<bool>("is_liquid", "True for liquid, false for vapor");
  params.addRequiredParam<Real>("vf_norm_factor", "Normalization factor for volume fraction equation.");

  params.addRequiredParam<UserObjectName>("eos", "The name of equation of state object to use.");

  return params;
}

OneDMomentumFlux::OneDMomentumFlux(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _is_liquid(getParam<bool>("is_liquid")),
    _sign(_is_liquid ? 1. : -1.),
    _alpha(coupledValue("alpha")),
    _rho(coupledValue("rho")),
    _rhou(coupledValue("rhou")),
    _rhoE(isCoupled("rhoE") ? coupledValue("rhoE") : _zero),
    _rhoA(coupledValue("rhoA")),
    _rhoEA(isCoupled("rhoEA") ? coupledValue("rhoEA") : _zero),
    _u_vel(coupledValue("u")),
    _pressure(coupledValue("pressure")),
    _area(coupledValue("area")),
    _rhoA_var_number(coupled("rhoA")),
    _rhoEA_var_number(isCoupled("rhoEA") ? coupled("rhoEA") : libMesh::invalid_uint),
    _has_alpha_A(isCoupled("alpha_A_liquid")),
    _alpha_A_liquid_var_number(_has_alpha_A ? coupled("alpha_A_liquid") : libMesh::invalid_uint),
    _dp_dalphaA_liquid(_has_alpha_A ?
        (_is_liquid ? &getMaterialProperty<Real>("dp_L_d_alphaA_L") : &getMaterialProperty<Real>("dp_V_d_alphaA_L")) :
        NULL),
    // volume fraction norm factor
    _vf_norm_factor(getParam<Real>("vf_norm_factor")),
    _eos(getUserObject<EquationOfState>("eos"))
{
}

OneDMomentumFlux::~OneDMomentumFlux()
{
}

Real
OneDMomentumFlux::computeQpResidual()
{
  Real F2 = _u[_qp] * _u_vel[_qp] + _alpha[_qp] * _pressure[_qp] * _area[_qp];
  // The contribution due to the convective flux.  Negative sign on
  // the F2 term comes from integration by parts.
  return -F2 * _grad_test[_i][_qp](0);

}

Real
OneDMomentumFlux::computeQpJacobian()
{
  // Derivatives wrt rho*u
  Real dp_drhou = _eos.dp_drhou(_rho[_qp], _rhou[_qp], _rhoE[_qp]);

  // (2,2) entry of flux Jacobian is the same as the constant area case, p_1 + 2*u
  Real A22 = 2. * _u_vel[_qp] + dp_drhou;

  // Negative sign comes from integration by parts
  return -A22 * _phi[_j][_qp] * _grad_test[_i][_qp](0);
}

Real
OneDMomentumFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _rhoA_var_number)
  {
    // Derivatives wrt rho
    Real dp_drho = _eos.dp_drho(_rho[_qp], _rhou[_qp], _rhoE[_qp]);

    // (2,1) entry of flux Jacobian is the same as the constant area case, p_0 - u^2
    Real A21 = dp_drho - _u_vel[_qp] * _u_vel[_qp];

    // The contribution from the convective flux term.  Negative sign comes from integration by parts.
    return -A21 * _phi[_j][_qp] * _grad_test[_i][_qp](0);
  }
  else if (jvar == _rhoEA_var_number)
  {
    // Derivatives wrt rhoE
    Real dp_drhoE = _eos.dp_drhoE(_rho[_qp], _rhou[_qp], _rhoE[_qp]);

    // (2,3) entry of flux Jacobian is the same as the constant area case, p_2
    Real A23 = dp_drhoE;

    // Contribution due to convective flux.  Negative sign comes from integration by parts.
    return -A23 * _phi[_j][_qp] * _grad_test[_i][_qp](0);
  }
  else if (jvar == _alpha_A_liquid_var_number)
  {
    return -(_sign * _pressure[_qp] + _alpha[_qp] * _area[_qp] * (*_dp_dalphaA_liquid)[_qp]) / _vf_norm_factor * _phi[_j][_qp] * _grad_test[_i][_qp](0);
  }
  else
    return 0.;
}
