#include "OneD7EqnEntropyMinimumArtificialDissipation.h"
#include "THMApp.h"
#include "MooseVariable.h"
#include "MooseMesh.h"

registerMooseObject("THMApp", OneD7EqnEntropyMinimumArtificialDissipation);

template <>
InputParameters
validParams<OneD7EqnEntropyMinimumArtificialDissipation>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<MooseEnum>("eqn_name",
                                     FlowModel::getFlowEquationType(),
                                     "The name of the equation this BC is acting on");
  params.addRequiredParam<bool>("is_liquid", "Boolean for phase: true-> liquid, false->vapor");
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addRequiredCoupledVar("rho", "Density of the phase");
  params.addRequiredCoupledVar("e", "Specific internal energy of the phase");
  params.addRequiredCoupledVar("vel", "Velocity of the phase");
  params.addRequiredCoupledVar("beta", "Remapped volume fraction of liquid (two-phase only)");
  params.addRequiredCoupledVar("arhoA", "Conserved density of the phase");
  params.addRequiredCoupledVar("arhouA", "Conserved momentum of the phase");
  params.addRequiredCoupledVar("arhoEA", "Conserved total energy of the phase");
  params.addRequiredParam<MaterialPropertyName>("alpha", "Volume fraction material property");
  params.addRequiredParam<MaterialPropertyName>("visc_beta",
                                                "The name of the viscous coefficient beta");
  params.addRequiredParam<MaterialPropertyName>("visc_mu",
                                                "The name of the viscous coefficient mu");
  params.addRequiredParam<MaterialPropertyName>("visc_kappa",
                                                "The name of the viscous coefficient kappa");
  return params;
}

OneD7EqnEntropyMinimumArtificialDissipation::OneD7EqnEntropyMinimumArtificialDissipation(
    const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Kernel>(parameters),
    _eqn_type(THM::stringToEnum<FlowModel::EEquationType>(getParam<MooseEnum>("eqn_name"))),

    _beta(coupledValue("beta")),

    _area(coupledValue("A")),
    _grad_area(coupledGradient("A")),

    _alpha(getMaterialProperty<Real>("alpha")),
    _dalpha_dbeta(getMaterialPropertyDerivativeTHM<Real>("alpha", "beta")),

    _rho(coupledValue("rho")),
    _grad_rho(coupledGradient("rho")),
    _e(coupledValue("e")),
    _grad_e(coupledGradient("e")),
    _vel(coupledValue("vel")),
    _grad_vel(coupledGradient("vel")),

    _arhoA(coupledValue("arhoA")),
    _grad_arhoA(coupledGradient("arhoA")),
    _arhouA(coupledValue("arhouA")),
    _grad_arhouA(coupledGradient("arhouA")),
    _grad_beta(coupledGradient("beta")),

    _is_liquid(getParam<bool>("is_liquid")),
    _sign(_is_liquid ? 1 : -1),
    _visc_kappa(getMaterialProperty<Real>("visc_kappa")),
    _visc_mu(getMaterialProperty<Real>("visc_mu")),
    _visc_beta(getMaterialProperty<Real>("visc_beta")),

    _beta_var_num(coupled("beta")),
    _arhoA_var_num(coupled("arhoA")),
    _arhouA_var_num(coupled("arhouA")),
    _arhoEA_var_num(coupled("arhoEA"))
{
}

RealVectorValue
OneD7EqnEntropyMinimumArtificialDissipation::l()
{
  return _area[_qp] * _visc_beta[_qp] * _dalpha_dbeta[_qp] * _grad_beta[_qp];
}

RealVectorValue
OneD7EqnEntropyMinimumArtificialDissipation::dl_dbeta()
{
  return _area[_qp] * _visc_beta[_qp] * _grad_phi[_j][_qp] * _dalpha_dbeta[_qp];
}

RealVectorValue
OneD7EqnEntropyMinimumArtificialDissipation::f()
{
  return _area[_qp] * _visc_kappa[_qp] * _alpha[_qp] * _grad_rho[_qp] + _rho[_qp] * l();
}

RealVectorValue
OneD7EqnEntropyMinimumArtificialDissipation::df_dbeta()
{
  // If we're in the liquid phase, we can just use the available aux variables.
  // In the vapor phase, we have:
  // grad(alpha_vapor*A) = grad( (1-alpha_liquid)*A )
  //                     = grad(A) - grad(alpha_liquid*A)
  //                     = _grad_area[_qp] - _grad_alpha_A_liquid[_qp]
  return
      // f_dissip-specific terms
      _sign * _visc_kappa[_qp] * _rho[_qp] *
          (-_grad_phi[_j][_qp] +
           _dalpha_dbeta[_qp] * _grad_beta[_qp] / _alpha[_qp] * _phi[_j][_qp]) *
          _area[_qp]

      // l_dissip terms
      - _sign * _rho[_qp] / _alpha[_qp] * _phi[_j][_qp] * l() + _rho[_qp] * dl_dbeta();
}

RealVectorValue
OneD7EqnEntropyMinimumArtificialDissipation::df_darhoA()
{
  // See comment above
  Real alpha_A = _alpha[_qp] * _area[_qp];

  return _visc_kappa[_qp] * (_grad_phi[_j][_qp] -
                             _dalpha_dbeta[_qp] * _grad_beta[_qp] / _alpha[_qp] * _phi[_j][_qp]) +
         l() / alpha_A * _phi[_j][_qp];
}

RealVectorValue
OneD7EqnEntropyMinimumArtificialDissipation::g()
{
  return _area[_qp] * _visc_mu[_qp] * _rho[_qp] * _alpha[_qp] * _grad_vel[_qp] + _vel[_qp] * f();
}

RealVectorValue
OneD7EqnEntropyMinimumArtificialDissipation::dg_dbeta()
{
  return _vel[_qp] * df_dbeta();
}

RealVectorValue
OneD7EqnEntropyMinimumArtificialDissipation::dg_darhoA()
{
  return _visc_mu[_qp] * (-_vel[_qp] * _grad_phi[_j][_qp] +
                          _vel[_qp] / _arhoA[_qp] * _grad_arhoA[_qp] * _phi[_j][_qp]) -
         f() * _vel[_qp] / _arhoA[_qp] * _phi[_j][_qp] + _vel[_qp] * df_darhoA();
}

RealVectorValue
OneD7EqnEntropyMinimumArtificialDissipation::dg_darhouA()
{
  return _visc_mu[_qp] * (_grad_phi[_j][_qp] - _phi[_j][_qp] / _arhoA[_qp] * _grad_arhoA[_qp]) +
         _phi[_j][_qp] / _arhoA[_qp] * f();
}

RealVectorValue
OneD7EqnEntropyMinimumArtificialDissipation::h()
{
  const RealVectorValue grad_rhoe = _grad_rho[_qp] * _e[_qp] + _rho[_qp] * _grad_e[_qp];
  return _area[_qp] * _visc_kappa[_qp] * _alpha[_qp] * grad_rhoe -
         0.5 * _vel[_qp] * _vel[_qp] * f() + _vel[_qp] * g() + _rho[_qp] * _e[_qp] * l();
}

RealVectorValue
OneD7EqnEntropyMinimumArtificialDissipation::dh_dbeta()
{
  // See comment above
  Real alpha_A = _alpha[_qp] * _area[_qp];

  return
      // h_dissip-specific terms
      _sign * _visc_kappa[_qp] *
          (-_rho[_qp] * _e[_qp] * _grad_phi[_j][_qp] + _rho[_qp] * _e[_qp] * _dalpha_dbeta[_qp] *
                                                           _grad_beta[_qp] / _alpha[_qp] *
                                                           _phi[_j][_qp]) *
          _area[_qp]

      // f_dissip terms
      - 0.5 * _vel[_qp] * _vel[_qp] * df_dbeta()

      // g_dissip terms
      + _vel[_qp] * dg_dbeta()

      // l_dissip terms
      + _rho[_qp] * _e[_qp] * dl_dbeta() + _sign * -_rho[_qp] * _e[_qp] / alpha_A * l();
}

RealVectorValue
OneD7EqnEntropyMinimumArtificialDissipation::dh_darhoA()
{
  // See comment above
  RealVectorValue grad_alpha_A = _dalpha_dbeta[_qp] * _grad_beta[_qp] * _area[_qp];
  Real alpha_A = _alpha[_qp] * _area[_qp];

  // We'll use this several times below
  Real u2 = _vel[_qp] * _vel[_qp];

  return
      // h_dissip-specific terms
      _visc_kappa[_qp] *
          (_vel[_qp] / _arhoA[_qp] * _grad_arhouA[_qp] * _phi[_j][_qp] +
           0.5 * u2 * (_grad_phi[_j][_qp] + grad_alpha_A / alpha_A * _phi[_j][_qp]) -
           u2 / _arhoA[_qp] * (_grad_arhoA[_qp] + _arhoA[_qp] / alpha_A * grad_alpha_A) *
               _phi[_j][_qp])

      // f_dissip terms
      - 0.5 * u2 * (df_darhoA() + f() / _arhoA[_qp] * _phi[_j][_qp])

      // g_dissip terms
      + _vel[_qp] * (dg_darhoA() - g() / _arhoA[_qp] * _phi[_j][_qp])

      // l_dissip terms
      + l() * 0.5 * u2 / alpha_A * _phi[_j][_qp];
}

RealVectorValue
OneD7EqnEntropyMinimumArtificialDissipation::dh_darhouA()
{
  // See comment above
  RealVectorValue grad_alpha_A = _dalpha_dbeta[_qp] * _grad_beta[_qp] * _area[_qp];
  Real alpha_A = _alpha[_qp] * _area[_qp];

  return
      // h_dissip-specific terms
      _visc_kappa[_qp] *
          (-_vel[_qp] * _grad_phi[_j][_qp] - _grad_arhouA[_qp] / _arhoA[_qp] * _phi[_j][_qp] +
           _vel[_qp] / _arhoA[_qp] * (_grad_arhoA[_qp] + _arhoA[_qp] / alpha_A * grad_alpha_A) *
               _phi[_j][_qp])

      // f_dissip terms
      - (_vel[_qp] / _arhoA[_qp]) * f() * _phi[_j][_qp]

      // g_dissip terms
      + g() / _arhoA[_qp] * _phi[_j][_qp] +
      _vel[_qp] * dg_darhouA()

      // l_dissip terms
      - _vel[_qp] / alpha_A * _phi[_j][_qp] * l();
}

RealVectorValue
OneD7EqnEntropyMinimumArtificialDissipation::dh_darhoEA()
{
  // See comment above
  RealVectorValue grad_alpha_A = _dalpha_dbeta[_qp] * _grad_beta[_qp] * _area[_qp];
  Real alpha_A = _alpha[_qp] * _area[_qp];

  return
      // h_dissip-specific terms
      _visc_kappa[_qp] * (_grad_phi[_j][_qp] - grad_alpha_A / alpha_A * _phi[_j][_qp])

      // l_dissip terms
      + l() / alpha_A * _phi[_j][_qp];
}

Real
OneD7EqnEntropyMinimumArtificialDissipation::computeQpResidual()
{
  if (!_mesh.getMesh().boundary_info->has_boundary_id(_current_elem->node_ptr(_i),
                                                      THM::bnd_nodeset_id))
  {
    switch (_eqn_type)
    {
      case FlowModel::VOIDFRACTION:
        return l() * _grad_test[_i][_qp];
      case FlowModel::CONTINUITY:
        return f() * _grad_test[_i][_qp];
      case FlowModel::MOMENTUM:
        return g() * _grad_test[_i][_qp];
      case FlowModel::ENERGY:
        return h() * _grad_test[_i][_qp];
      default:
        mooseError("The equation is not supported");
    }
  }
  else
    return 0;
}

Real
OneD7EqnEntropyMinimumArtificialDissipation::computeQpJacobian()
{
  if (!_mesh.getMesh().boundary_info->has_boundary_id(_current_elem->node_ptr(_i),
                                                      THM::bnd_nodeset_id))
  {
    switch (_eqn_type)
    {
      case FlowModel::VOIDFRACTION:
        return computeQpJacobianVolumeFraction(_var.number());
      case FlowModel::CONTINUITY:
        return computeQpJacobianDensity(_var.number());
      case FlowModel::MOMENTUM:
        return computeQpJacobianMomentum(_var.number());
      case FlowModel::ENERGY:
        return computeQpJacobianEnergy(_var.number());
      default:
        mooseError("The equation is not supported");
    }
  }
  else
    return 0.;
}

Real
OneD7EqnEntropyMinimumArtificialDissipation::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (!_mesh.getMesh().boundary_info->has_boundary_id(_current_elem->node_ptr(_i),
                                                      THM::bnd_nodeset_id))
  {
    switch (_eqn_type)
    {
      case FlowModel::VOIDFRACTION:
        return computeQpJacobianVolumeFraction(jvar);
      case FlowModel::CONTINUITY:
        return computeQpJacobianDensity(jvar);
      case FlowModel::MOMENTUM:
        return computeQpJacobianMomentum(jvar);
      case FlowModel::ENERGY:
        return computeQpJacobianEnergy(jvar);
      default:
        return 0;
    }
  }
  else
    return 0;
}

Real
OneD7EqnEntropyMinimumArtificialDissipation::computeQpJacobianVolumeFraction(unsigned int jvar)
{
  if (jvar == _beta_var_num)
    return dl_dbeta() * _grad_test[_i][_qp];
  else
    return 0;
}

Real
OneD7EqnEntropyMinimumArtificialDissipation::computeQpJacobianDensity(unsigned int jvar)
{
  if (jvar == _beta_var_num)
    return df_dbeta() * _grad_test[_i][_qp];
  else if (jvar == _arhoA_var_num)
    return df_darhoA() * _grad_test[_i][_qp];
  else
    return 0;
}

Real
OneD7EqnEntropyMinimumArtificialDissipation::computeQpJacobianMomentum(unsigned int jvar)
{
  if (jvar == _beta_var_num)
    return dg_dbeta() * _grad_test[_i][_qp];
  else if (jvar == _arhoA_var_num)
    return dg_darhoA() * _grad_test[_i][_qp];
  else if (jvar == _arhouA_var_num)
    return dg_darhouA() * _grad_test[_i][_qp];
  else if (jvar == _arhoEA_var_num)
    return 0;
  else
    return 0;
}

Real
OneD7EqnEntropyMinimumArtificialDissipation::computeQpJacobianEnergy(unsigned int jvar)
{
  if (jvar == _beta_var_num)
    return dh_dbeta() * _grad_test[_i][_qp];
  else if (jvar == _arhoA_var_num)
    return dh_darhoA() * _grad_test[_i][_qp];
  else if (jvar == _arhouA_var_num)
    return dh_darhouA() * _grad_test[_i][_qp];
  else if (jvar == _arhoEA_var_num)
    return dh_darhoEA() * _grad_test[_i][_qp];
  else
    return 0;
}
