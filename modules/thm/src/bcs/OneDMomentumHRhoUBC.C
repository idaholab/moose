#include "OneDMomentumHRhoUBC.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("THMApp", OneDMomentumHRhoUBC);

template <>
InputParameters
validParams<OneDMomentumHRhoUBC>()
{
  InputParameters params = validParams<OneDIntegratedBC>();
  params.addParam<bool>("is_liquid", true, "true for liquid, false for vapor");
  params.addRequiredParam<Real>("rhou", "Specified momentum");

  params.addCoupledVar("alpha", 1., "Volume fraction of the phase");
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addCoupledVar("beta", "Remapped volume fraction of liquid (two-phase only)");
  params.addRequiredCoupledVar("arhoA", "");
  params.addRequiredCoupledVar("arhouA", "");
  params.addRequiredCoupledVar("arhoEA", "");

  params.addRequiredParam<MaterialPropertyName>("p", "Pressure property name");

  params.declareControllable("rhou");

  return params;
}

OneDMomentumHRhoUBC::OneDMomentumHRhoUBC(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<OneDIntegratedBC>(parameters),
    _is_liquid(getParam<bool>("is_liquid")),
    _sign(_is_liquid ? 1. : -1.),
    _rhou(getParam<Real>("rhou")),
    _alpha(coupledValue("alpha")),
    _area(coupledValue("A")),
    _arhoA(coupledValue("arhoA")),
    _arhoEA(coupledValue("arhoEA")),
    _p(getMaterialProperty<Real>("p")),
    _dp_darhoA(getMaterialPropertyDerivativeTHM<Real>("p", "arhoA")),
    _dp_darhouA(getMaterialPropertyDerivativeTHM<Real>("p", "arhouA")),
    _dp_darhoEA(getMaterialPropertyDerivativeTHM<Real>("p", "arhoEA")),
    _dp_dbeta(isCoupled("beta") ? &getMaterialPropertyDerivativeTHM<Real>("p", "beta") : nullptr),

    _arhoA_var_num(coupled("arhoA")),
    _arhoEA_var_num(coupled("arhoEA")),
    _beta_var_num(isCoupled("beta") ? coupled("beta") : libMesh::invalid_uint)
{
}

Real
OneDMomentumHRhoUBC::computeQpResidual()
{
  Real arhouA = _alpha[_qp] * _rhou * _area[_qp];
  return (arhouA * arhouA / _arhoA[_qp] + _alpha[_qp] * _p[_qp] * _area[_qp]) * _normal *
         _test[_i][_qp];
}

Real
OneDMomentumHRhoUBC::computeQpJacobian()
{
  return _alpha[_qp] * _dp_darhouA[_qp] * _area[_qp] * _normal * _phi[_j][_qp] * _test[_i][_qp];
}

Real
OneDMomentumHRhoUBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _arhoA_var_num)
  {
    Real arhouA = _alpha[_qp] * _rhou * _area[_qp];
    return (-arhouA * arhouA / _arhoA[_qp] / _arhoA[_qp] +
            _alpha[_qp] * _dp_darhoA[_qp] * _area[_qp]) *
           _normal * _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _arhoEA_var_num)
  {
    return _alpha[_qp] * _dp_darhoEA[_qp] * _area[_qp] * _normal * _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _beta_var_num)
  {
    Real alpha_A = _alpha[_qp] * _area[_qp];
    return (_sign * 2 * alpha_A * _rhou * _rhou / _arhoA[_qp] +
            (_sign * _p[_qp] + _alpha[_qp] * (*_dp_dbeta)[_qp])) *
           _area[_qp] * _normal * _phi[_j][_qp] * _test[_i][_qp];
  }
  else
    return 0;
}
