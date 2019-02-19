#include "OneDEntropyMinimumArtificialDissipation.h"
#include "FlowModel.h"
#include "THMApp.h"
#include "MooseVariable.h"
#include "MooseMesh.h"

registerMooseObject("THMApp", OneDEntropyMinimumArtificialDissipation);

template <>
InputParameters
validParams<OneDEntropyMinimumArtificialDissipation>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<MooseEnum>("eqn_name",
                                     FlowModel::getFlowEquationType(),
                                     "The name of the equation this BC is acting on");
  params.addRequiredCoupledVar("rhoA", "Conserved density");
  params.addRequiredCoupledVar("rhouA", "Conserved momentum");
  params.addRequiredCoupledVar("rhoEA", "Conserved total energy");
  params.addRequiredCoupledVar("rho", "Density");
  params.addRequiredCoupledVar("e", "Specific internal energy");
  params.addRequiredCoupledVar("vel", "The x-component of the velocity");
  params.addRequiredCoupledVar("A", "Cross-sectional variable");
  params.addRequiredParam<MaterialPropertyName>("visc_mu",
                                                "The name of the viscous coefficient mu");
  params.addRequiredParam<MaterialPropertyName>("visc_kappa",
                                                "The name of the viscous coefficient kappa");

  return params;
}

OneDEntropyMinimumArtificialDissipation::OneDEntropyMinimumArtificialDissipation(
    const InputParameters & parameters)
  : Kernel(parameters),
    _eqn_type(THM::stringToEnum<FlowModel::EEquationType>(getParam<MooseEnum>("eqn_name"))),
    // Variables
    _rhoA(coupledValue("rhoA")),
    _grad_rhoA(coupledGradient("rhoA")),
    _rhouA(coupledValue("rhouA")),
    _grad_rhouA(coupledGradient("rhouA")),
    _rhoEA(coupledValue("rhoEA")),
    _grad_rhoEA(coupledGradient("rhoEA")),
    _area(coupledValue("A")),
    _grad_area(coupledGradient("A")),
    _rho(coupledValue("rho")),
    _grad_rho(coupledGradient("rho")),
    _e(coupledValue("e")),
    _grad_e(coupledGradient("e")),
    _velocity(coupledValue("vel")),
    _grad_velocity(coupledGradient("vel")),
    // Material
    _mu(getMaterialProperty<Real>("visc_mu")),
    _kappa(getMaterialProperty<Real>("visc_kappa")),
    // Integers for off-diag jack terms
    _rhoA_var_num(coupled("rhoA")),
    _rhouA_var_num(coupled("rhouA")),
    _rhoEA_var_num(coupled("rhoEA"))
{
}

RealVectorValue
OneDEntropyMinimumArtificialDissipation::f()
{
  return _area[_qp] * _kappa[_qp] * _grad_rho[_qp];
}

RealVectorValue
OneDEntropyMinimumArtificialDissipation::df_drhoA()
{
  return _kappa[_qp] * (_grad_phi[_j][_qp] - _phi[_j][_qp] / _area[_qp] * _grad_area[_qp]);
}

RealVectorValue
OneDEntropyMinimumArtificialDissipation::g()
{
  return _area[_qp] * _mu[_qp] * _rho[_qp] * _grad_velocity[_qp] + _velocity[_qp] * f();
}

RealVectorValue
OneDEntropyMinimumArtificialDissipation::dg_drhoA()
{
  // NOTE: mu depends on rhoA, rhouA, rhoEA, but we treat it as a constant
  return -_mu[_qp] * (_velocity[_qp] * _grad_phi[_j][_qp] -
                      _velocity[_qp] / _rhoA[_qp] * _grad_rhoA[_qp] * _phi[_j][_qp]) +
         (_velocity[_qp] * df_drhoA() - _velocity[_qp] / _rhoA[_qp] * f() * _phi[_j][_qp]);
}

RealVectorValue
OneDEntropyMinimumArtificialDissipation::dg_drhouA()
{
  // NOTE: mu depends on rhoA, rhouA, rhoEA, but we treat it as a constant
  return _mu[_qp] * _grad_phi[_j][_qp] - _mu[_qp] * _grad_rhoA[_qp] / _rhoA[_qp] * _phi[_j][_qp] +
         f() / _rhoA[_qp] * _phi[_j][_qp];
}

RealVectorValue
OneDEntropyMinimumArtificialDissipation::h()
{
  // NOTE: kappa depends on rhoA, rhouA, rhoEA, but we treat it as a constant
  const RealVectorValue grad_rhoe = _grad_rho[_qp] * _e[_qp] + _rho[_qp] * _grad_e[_qp];
  return _area[_qp] * _kappa[_qp] * grad_rhoe - 0.5 * _velocity[_qp] * _velocity[_qp] * f() +
         _velocity[_qp] * g();
}

RealVectorValue
OneDEntropyMinimumArtificialDissipation::dh_drhoA()
{
  // NOTE: kappa depends on rhoA, rhouA, rhoEA, but we treat it as a constant
  Real u = _velocity[_qp];
  Real u2 = u * u;

  return _kappa[_qp] *
             (u / _rhoA[_qp] * _grad_rhouA[_qp] * _phi[_j][_qp] -
              u2 / _rhoA[_qp] * _grad_rhoA[_qp] * _phi[_j][_qp] + 0.5 * u2 * _grad_phi[_j][_qp] -
              0.5 * u2 / _area[_qp] * _grad_area[_qp] * _phi[_j][_qp]) +
         u2 / _rhoA[_qp] * f() * _phi[_j][_qp] - 0.5 * u2 * df_drhoA() -
         u / _rhoA[_qp] * g() * _phi[_j][_qp] + u * dg_drhoA();
}

RealVectorValue
OneDEntropyMinimumArtificialDissipation::dh_drhouA()
{
  // NOTE: kappa depends on rhoA, rhouA, rhoEA, but we treat it as a constant
  Real u = _velocity[_qp];
  return _kappa[_qp] * (-_grad_rhouA[_qp] / _rhoA[_qp] * _phi[_j][_qp] - u * _grad_phi[_j][_qp] +
                        u / _rhoA[_qp] * _grad_rhoA[_qp] * _phi[_j][_qp] +
                        u / _area[_qp] * _grad_area[_qp] * _phi[_j][_qp]) -
         u / _rhoA[_qp] * f() * _phi[_j][_qp] + g() / _rhoA[_qp] * _phi[_j][_qp] + u * dg_drhouA();
}

RealVectorValue
OneDEntropyMinimumArtificialDissipation::dh_drhoEA()
{
  // NOTE: kappa depends on rhoA, rhouA, rhoEA, but we treat it as a constant
  return _kappa[_qp] * (_grad_phi[_j][_qp] - _grad_area[_qp] / _area[_qp] * _phi[_j][_qp]);
}

Real
OneDEntropyMinimumArtificialDissipation::computeQpResidual()
{
  if (!_mesh.getMesh().boundary_info->has_boundary_id(_current_elem->node_ptr(_i),
                                                      THM::bnd_nodeset_id))
  {
    switch (_eqn_type)
    {
      case FlowModel::CONTINUITY:
        return f() * _grad_test[_i][_qp];
      case FlowModel::MOMENTUM:
        return g() * _grad_test[_i][_qp];
      case FlowModel::ENERGY:
        return h() * _grad_test[_i][_qp];
      default:
        return 0;
    }
  }
  else
    return 0;
}

Real
OneDEntropyMinimumArtificialDissipation::computeQpJacobian()
{
  if (!_mesh.getMesh().boundary_info->has_boundary_id(_current_elem->node_ptr(_i),
                                                      THM::bnd_nodeset_id))
  {
    switch (_eqn_type)
    {
      case FlowModel::CONTINUITY:
        return computeQpJacobianDensity(_var.number());
      case FlowModel::MOMENTUM:
        return computeQpJacobianMomentum(_var.number());
      case FlowModel::ENERGY:
        return computeQpJacobianEnergy(_var.number());
      default:
        return 0;
    }
  }
  else
    return 0;
}

Real
OneDEntropyMinimumArtificialDissipation::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (!_mesh.getMesh().boundary_info->has_boundary_id(_current_elem->node_ptr(_i),
                                                      THM::bnd_nodeset_id))
  {
    switch (_eqn_type)
    {
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
OneDEntropyMinimumArtificialDissipation::computeQpJacobianDensity(unsigned int jvar)
{
  if (jvar == _rhoA_var_num)
    return df_drhoA() * _grad_test[_i][_qp];
  else
    return 0;
}

Real
OneDEntropyMinimumArtificialDissipation::computeQpJacobianMomentum(unsigned int jvar)
{
  if (jvar == _rhoA_var_num)
    return dg_drhoA() * _grad_test[_i][_qp];
  else if (jvar == _rhouA_var_num)
    return dg_drhouA() * _grad_test[_i][_qp];
  else
    return 0.;
}

Real
OneDEntropyMinimumArtificialDissipation::computeQpJacobianEnergy(unsigned int jvar)
{
  if (jvar == _rhoA_var_num)
    return dh_drhoA() * _grad_test[_i][_qp];
  else if (jvar == _rhouA_var_num)
    return dh_drhouA() * _grad_test[_i][_qp];
  else if (jvar == _rhoEA_var_num)
    return dh_drhoEA() * _grad_test[_i][_qp];
  else
    return 0.;
}
