#include "OneDMomentumAreaGradient.h"

registerMooseObject("RELAP7App", OneDMomentumAreaGradient);

template <>
InputParameters
validParams<OneDMomentumAreaGradient>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("arhoA", "The density of the kth phase");
  params.addRequiredCoupledVar("arhouA", "The momentum of the kth phase");
  params.addRequiredCoupledVar("arhoEA", "The total energy of the kth phase");
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addCoupledVar("beta", 0, "Remapped volume fraction of liquid (two-phase only)");
  params.addRequiredParam<MaterialPropertyName>("p", "Pressure");
  params.addRequiredParam<MaterialPropertyName>("alpha", "Volume fraction material property");
  return params;
}

OneDMomentumAreaGradient::OneDMomentumAreaGradient(const InputParameters & parameters)
  : DerivativeMaterialInterfaceRelap<Kernel>(parameters),
    _alpha(getMaterialProperty<Real>("alpha")),
    _dalpha_dbeta(isCoupled("beta") ? &getMaterialPropertyDerivativeRelap<Real>("alpha", "beta")
                                    : nullptr),

    _area(coupledValue("A")),
    _area_grad(coupledGradient("A")),
    _pressure(getMaterialProperty<Real>("p")),
    _dp_dbeta(isCoupled("beta") ? &getMaterialPropertyDerivativeRelap<Real>("p", "beta") : nullptr),
    _dp_darhoA(getMaterialPropertyDerivativeRelap<Real>("p", "arhoA")),
    _dp_darhouA(getMaterialPropertyDerivativeRelap<Real>("p", "arhouA")),
    _dp_darhoEA(getMaterialPropertyDerivativeRelap<Real>("p", "arhoEA")),

    _arhoA_var_number(coupled("arhoA")),
    _arhoE_var_number(coupled("arhoEA")),
    _beta_var_number(isCoupled("beta") ? coupled("beta") : libMesh::invalid_uint)
{
}

Real
OneDMomentumAreaGradient::computeQpResidual()
{
  return -_pressure[_qp] * _alpha[_qp] * _area_grad[_qp](0) * _test[_i][_qp];
}

Real
OneDMomentumAreaGradient::computeQpJacobian()
{
  return -_dp_darhouA[_qp] * _alpha[_qp] * _area_grad[_qp](0) * _phi[_j][_qp] * _test[_i][_qp];
}

Real
OneDMomentumAreaGradient::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _arhoA_var_number)
  {
    return -_dp_darhoA[_qp] * _alpha[_qp] * _area_grad[_qp](0) * _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _arhoE_var_number)
  {
    return -_dp_darhoEA[_qp] * _alpha[_qp] * _area_grad[_qp](0) * _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _beta_var_number)
  {
    return -((*_dp_dbeta)[_qp] * _alpha[_qp] + _pressure[_qp] * (*_dalpha_dbeta)[_qp]) *
           _area_grad[_qp](0) * _phi[_j][_qp] * _test[_i][_qp];
  }
  else
    return 0;
}
