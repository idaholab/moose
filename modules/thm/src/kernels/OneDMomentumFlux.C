#include "OneDMomentumFlux.h"

template<>
InputParameters validParams<OneDMomentumFlux>()
{
  InputParameters params = validParams<Kernel>();
  params.addCoupledVar("alpha", 1.0, "Volume fraction");
  params.addCoupledVar("alpha_A_liquid", "Volume fraction of liquid");
  params.addRequiredCoupledVar("rhoA", "density multiplied by area");
  params.addCoupledVar("rhoEA", "total energy multiplied by area");
  params.addRequiredCoupledVar("u", "velocity");
  params.addRequiredCoupledVar("area", "cross-sectional area");
  params.addParam<bool>("is_liquid", true, "True for liquid, false for vapor");

  return params;
}

OneDMomentumFlux::OneDMomentumFlux(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _is_liquid(getParam<bool>("is_liquid")),
    _sign(_is_liquid ? 1. : -1.),
    _alpha(coupledValue("alpha")),
    _u_vel(coupledValue("u")),
    _pressure(_is_liquid ? getMaterialProperty<Real>("pressure_liquid") : getMaterialProperty<Real>("pressure_vapor")),
    _dp_drho(_is_liquid ? getMaterialProperty<Real>("dpL_drho") : getMaterialProperty<Real>("dpV_drho")),
    _dp_drhou(_is_liquid ? getMaterialProperty<Real>("dpL_drhou") : getMaterialProperty<Real>("dpV_drhou")),
    _dp_drhoE(_is_liquid ? getMaterialProperty<Real>("dpL_drhoE") : getMaterialProperty<Real>("dpV_drhoE")),
    _area(coupledValue("area")),
    _rhoA_var_number(coupled("rhoA")),
    _rhoEA_var_number(isCoupled("rhoEA") ? coupled("rhoEA") : libMesh::invalid_uint),
    _has_alpha_A(isCoupled("alpha_A_liquid")),
    _alpha_A_liquid_var_number(_has_alpha_A ? coupled("alpha_A_liquid") : libMesh::invalid_uint),
    _dp_dalphaA_liquid(_has_alpha_A ?
        (_is_liquid ? &getMaterialProperty<Real>("dp_L_d_alphaA_L") : &getMaterialProperty<Real>("dp_V_d_alphaA_L")) :
        NULL)
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
  // (2,2) entry of flux Jacobian is the same as the constant area case, p_1 + 2*u
  Real A22 = 2. * _u_vel[_qp] + _dp_drhou[_qp];

  // Negative sign comes from integration by parts
  return -A22 * _phi[_j][_qp] * _grad_test[_i][_qp](0);
}

Real
OneDMomentumFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _rhoA_var_number)
  {
    // (2,1) entry of flux Jacobian is the same as the constant area case, p_0 - u^2
    Real A21 = _dp_drho[_qp] - _u_vel[_qp] * _u_vel[_qp];

    // The contribution from the convective flux term.  Negative sign comes from integration by parts.
    return -A21 * _phi[_j][_qp] * _grad_test[_i][_qp](0);
  }
  else if (jvar == _rhoEA_var_number)
  {
    // (2,3) entry of flux Jacobian is the same as the constant area case, p_2
    Real A23 = _dp_drhoE[_qp];

    // Contribution due to convective flux.  Negative sign comes from integration by parts.
    return -A23 * _phi[_j][_qp] * _grad_test[_i][_qp](0);
  }
  else if (jvar == _alpha_A_liquid_var_number)
  {
    return -(_sign * _pressure[_qp] + _alpha[_qp] * _area[_qp] * (*_dp_dalphaA_liquid)[_qp]) * _phi[_j][_qp] * _grad_test[_i][_qp](0);
  }
  else
    return 0.;
}
