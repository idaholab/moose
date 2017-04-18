#include "OneDMomentumAreaGradient.h"

template <>
InputParameters
validParams<OneDMomentumAreaGradient>()
{
  InputParameters params = validParams<Kernel>();
  params.addParam<bool>("is_liquid", false, "True for liquid phase, false for vapor");
  params.addRequiredCoupledVar("arhoA", "The density of the kth phase");
  params.addRequiredCoupledVar("arhouA", "The momentum of the kth phase");
  params.addCoupledVar("arhoEA", "The total energy of the kth phase");
  params.addRequiredCoupledVar("area", "Cross-sectional area");
  params.addCoupledVar("alpha", 1, "The volume fraction of the kth phase");
  params.addCoupledVar("beta", 0, "Remapped volume fraction of liquid (two-phase only)");
  params.addRequiredParam<MaterialPropertyName>("pressure", "Pressure");
  params.addParam<MaterialPropertyName>("daL_dbeta", "Derivative of alphaL w.r.t. beta");
  return params;
}

OneDMomentumAreaGradient::OneDMomentumAreaGradient(const InputParameters & parameters)
  : DerivativeMaterialInterfaceRelap<Kernel>(parameters),
    _is_liquid(getParam<bool>("is_liquid")),
    _sign(_is_liquid ? 1. : -1.),
    _alpha(coupledValue("alpha")),
    _area(coupledValue("area")),
    _area_grad(coupledGradient("area")),
    _pressure(getMaterialProperty<Real>("pressure")),
    _dp_dbeta(isCoupled("beta") ? &getMaterialPropertyDerivativeRelap<Real>("pressure", "beta")
                                : NULL),
    _dp_darhoA(getMaterialPropertyDerivativeRelap<Real>("pressure", "arhoA")),
    _dp_darhouA(getMaterialPropertyDerivativeRelap<Real>("pressure", "arhouA")),
    _dp_darhoEA(isCoupled("arhoEA")
                    ? &getMaterialPropertyDerivativeRelap<Real>("pressure", "arhoEA")
                    : nullptr),
    _daL_dbeta(isCoupled("beta") ? &getMaterialProperty<Real>("daL_dbeta") : nullptr),
    _alpha_rhoA_var_number(coupled("arhoA")),
    _alpha_rhoE_var_number(isCoupled("arhoEA") ? coupled("arhoEA") : libMesh::invalid_uint),
    _beta_var_number(isCoupled("beta") ? coupled("beta") : libMesh::invalid_uint)
{
}

OneDMomentumAreaGradient::~OneDMomentumAreaGradient() {}

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
  if (jvar == _alpha_rhoA_var_number)
  {
    return -_dp_darhoA[_qp] * _alpha[_qp] * _area_grad[_qp](0) * _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _alpha_rhoE_var_number)
  {
    return -(*_dp_darhoEA)[_qp] * _alpha[_qp] * _area_grad[_qp](0) * _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _beta_var_number)
  {
    return -((*_dp_dbeta)[_qp] * _alpha[_qp] + _sign * _pressure[_qp] * (*_daL_dbeta)[_qp]) *
           _area_grad[_qp](0) * _phi[_j][_qp] * _test[_i][_qp];
  }
  else
    return 0;
}
