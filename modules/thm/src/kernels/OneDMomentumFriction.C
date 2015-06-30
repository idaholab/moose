#include "OneDMomentumFriction.h"

template<>
InputParameters validParams<OneDMomentumFriction>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("rhoA", "density term");
  params.addRequiredCoupledVar("rhouA", "momentum term");
  params.addRequiredCoupledVar("u", "velocity");
  params.addRequiredCoupledVar("hydraulic_diameter", "The hydraulic diameter. Depends on A(x).");
  return params;
}

OneDMomentumFriction::OneDMomentumFriction(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _u_vel(coupledValue("u")),
    _rhouA(coupledValue("rhouA")),
    _hydraulic_diameter(coupledValue("hydraulic_diameter")),
    _rhoA_var_number(coupled("rhoA")),
    _friction(getMaterialPropertyByName<Real>("friction"))
{
}

OneDMomentumFriction::~OneDMomentumFriction()
{
}


Real
OneDMomentumFriction::computeQpResidual()
{
  return (0.5 * _friction[_qp] / _hydraulic_diameter[_qp]) * _rhouA[_qp] * std::abs(_u_vel[_qp]) * _test[_i][_qp];
}


Real
OneDMomentumFriction::computeQpJacobian()
{
  return (0.5 * _friction[_qp] / _hydraulic_diameter[_qp]) * 2. * std::abs(_u_vel[_qp]) * _phi[_j][_qp] * _test[_i][_qp];
}


Real
OneDMomentumFriction::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _rhoA_var_number)
    return (0.5 * _friction[_qp] / _hydraulic_diameter[_qp]) * (-_u_vel[_qp]) * std::abs(_u_vel[_qp]) * _phi[_j][_qp] * _test[_i][_qp];
  else
    return 0.;
}
