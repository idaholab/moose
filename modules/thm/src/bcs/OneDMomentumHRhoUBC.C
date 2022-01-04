#include "OneDMomentumHRhoUBC.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("THMApp", OneDMomentumHRhoUBC);

InputParameters
OneDMomentumHRhoUBC::validParams()
{
  InputParameters params = OneDIntegratedBC::validParams();
  params.addRequiredParam<Real>("rhou", "Specified momentum");
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addRequiredCoupledVar("arhoA", "");
  params.addRequiredCoupledVar("arhouA", "");
  params.addRequiredCoupledVar("arhoEA", "");

  params.addRequiredParam<MaterialPropertyName>("p", "Pressure property name");

  params.declareControllable("rhou");

  return params;
}

OneDMomentumHRhoUBC::OneDMomentumHRhoUBC(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<OneDIntegratedBC>(parameters),
    _rhou(getParam<Real>("rhou")),
    _area(coupledValue("A")),
    _arhoA(coupledValue("arhoA")),
    _arhoEA(coupledValue("arhoEA")),
    _p(getMaterialProperty<Real>("p")),
    _dp_darhoA(getMaterialPropertyDerivativeTHM<Real>("p", "arhoA")),
    _dp_darhouA(getMaterialPropertyDerivativeTHM<Real>("p", "arhouA")),
    _dp_darhoEA(getMaterialPropertyDerivativeTHM<Real>("p", "arhoEA")),

    _arhoA_var_num(coupled("arhoA")),
    _arhoEA_var_num(coupled("arhoEA"))
{
}

Real
OneDMomentumHRhoUBC::computeQpResidual()
{
  Real arhouA = _rhou * _area[_qp];
  return (arhouA * arhouA / _arhoA[_qp] + _p[_qp] * _area[_qp]) * _normal * _test[_i][_qp];
}

Real
OneDMomentumHRhoUBC::computeQpJacobian()
{
  return _dp_darhouA[_qp] * _area[_qp] * _normal * _phi[_j][_qp] * _test[_i][_qp];
}

Real
OneDMomentumHRhoUBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _arhoA_var_num)
  {
    Real arhouA = _rhou * _area[_qp];
    return (-arhouA * arhouA / _arhoA[_qp] / _arhoA[_qp] + _dp_darhoA[_qp] * _area[_qp]) * _normal *
           _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _arhoEA_var_num)
  {
    return _dp_darhoEA[_qp] * _area[_qp] * _normal * _phi[_j][_qp] * _test[_i][_qp];
  }
  else
    return 0;
}
