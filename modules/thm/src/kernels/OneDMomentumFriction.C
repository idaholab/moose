#include "OneDMomentumFriction.h"

template <>
InputParameters
validParams<OneDMomentumFriction>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("area", "Cross-sectional area");
  params.addRequiredCoupledVar("rhoA", "Conserved density");
  params.addRequiredCoupledVar("rhouA", "Conserved momentum");
  params.addCoupledVar("rhoEA", "Conserved total energy");
  params.addRequiredCoupledVar("u", "velocity");
  params.addCoupledVar("beta", "Remapped volume fraction of liquid (two-phase only)");
  params.addRequiredParam<MaterialPropertyName>(
      "Cw", "The name of the material property that stores the wall drag coefficient");
  return params;
}

OneDMomentumFriction::OneDMomentumFriction(const InputParameters & parameters)
  : DerivativeMaterialInterfaceRelap<Kernel>(parameters),
    _area(coupledValue("area")),
    _u_vel(coupledValue("u")),
    _rhoA(coupledValue("rhoA")),
    _Cw(getMaterialProperty<Real>("Cw")),
    _dCw_dbeta(isCoupled("beta") ? &getMaterialPropertyDerivativeRelap<Real>("Cw", "beta") : NULL),
    _dCw_drhoA(getMaterialPropertyDerivativeRelap<Real>("Cw", "rhoA")),
    _dCw_drhouA(getMaterialPropertyDerivativeRelap<Real>("Cw", "rhouA")),
    _dCw_drhoEA(isCoupled("rhoEA") ? &getMaterialPropertyDerivativeRelap<Real>("Cw", "rhoEA")
                                   : NULL),
    _beta_var_number(isCoupled("beta") ? coupled("beta") : libMesh::invalid_uint),
    _rhoA_var_number(coupled("rhoA")),
    _rhouA_var_number(coupled("rhouA")),
    _rhoEA_var_number(isCoupled("rhoEA") ? coupled("rhoEA") : libMesh::invalid_uint)
{
}

OneDMomentumFriction::~OneDMomentumFriction() {}

Real
OneDMomentumFriction::computeQpResidual()
{
  return _Cw[_qp] * _u_vel[_qp] * std::abs(_u_vel[_qp]) * _area[_qp] * _test[_i][_qp];
}

Real
OneDMomentumFriction::computeQpJacobian()
{
  return computeQpOffDiagJacobian(_var.number());
}

Real
OneDMomentumFriction::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _rhoA_var_number)
  {
    Real ddU0 = (_dCw_drhoA[_qp] * _u_vel[_qp] * std::abs(_u_vel[_qp]) -
                 2. * _Cw[_qp] * _u_vel[_qp] * std::abs(_u_vel[_qp]) / _rhoA[_qp]);
    return ddU0 * _area[_qp] * _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _rhouA_var_number)
  {
    Real ddU1 = (_dCw_drhouA[_qp] * _u_vel[_qp] * std::abs(_u_vel[_qp]) +
                 2. * _Cw[_qp] * std::abs(_u_vel[_qp]) / _rhoA[_qp]);
    return ddU1 * _area[_qp] * _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _rhoEA_var_number)
  {
    return (*_dCw_drhoEA)[_qp] * _u_vel[_qp] * std::abs(_u_vel[_qp]) * _area[_qp] * _phi[_j][_qp] *
           _test[_i][_qp];
  }
  else if (jvar == _beta_var_number)
  {
    return (*_dCw_dbeta)[_qp] * _u_vel[_qp] * std::abs(_u_vel[_qp]) * _area[_qp] * _phi[_j][_qp] *
           _test[_i][_qp];
  }
  else
    return 0;
}
