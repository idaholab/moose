#include "OneDMomentumAreaGradient.h"

template <>
InputParameters
validParams<OneDMomentumAreaGradient>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("rhoA", "Conserved density");
  params.addRequiredCoupledVar("rhouA", "Conserved momentum");
  params.addCoupledVar("rhoEA", "Conserved total energy");
  params.addRequiredCoupledVar("area", "Cross-sectional area");

  return params;
}

OneDMomentumAreaGradient::OneDMomentumAreaGradient(const InputParameters & parameters)
  : DerivativeMaterialInterfaceRelap<Kernel>(parameters),
    _pressure(getMaterialProperty<Real>("pressure")),
    _dp_drhoA(getMaterialPropertyDerivativeRelap<Real>("pressure", "rhoA")),
    _dp_drhouA(getMaterialPropertyDerivativeRelap<Real>("pressure", "rhouA")),
    _dp_drhoEA(isCoupled("rhoEA") ? &getMaterialPropertyDerivativeRelap<Real>("pressure", "rhoEA")
                                  : NULL),
    _area(coupledValue("area")),
    _area_grad(coupledGradient("area")),
    _rhoA_var_number(coupled("rhoA")),
    _rhoEA_var_number(isCoupled("rhoEA") ? coupled("rhoEA") : libMesh::invalid_uint)
{
}

OneDMomentumAreaGradient::~OneDMomentumAreaGradient() {}

Real
OneDMomentumAreaGradient::computeQpResidual()
{
  return -_pressure[_qp] * _area_grad[_qp](0) * _test[_i][_qp];
}

Real
OneDMomentumAreaGradient::computeQpJacobian()
{
  return -_dp_drhouA[_qp] * _area_grad[_qp](0) * _phi[_j][_qp] * _test[_i][_qp];
}

Real
OneDMomentumAreaGradient::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _rhoA_var_number)
  {
    return -_dp_drhoA[_qp] * _area_grad[_qp](0) * _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _rhoEA_var_number)
  {
    return -(*_dp_drhoEA)[_qp] * _area_grad[_qp](0) * _phi[_j][_qp] * _test[_i][_qp];
  }
  else
    return 0;
}
