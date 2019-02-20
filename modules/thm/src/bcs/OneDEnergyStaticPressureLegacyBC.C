#include "OneDEnergyStaticPressureLegacyBC.h"
#include "Numerics.h"

registerMooseObject("THMApp", OneDEnergyStaticPressureLegacyBC);

template <>
InputParameters
validParams<OneDEnergyStaticPressureLegacyBC>()
{
  InputParameters params = validParams<OneDIntegratedBC>();
  params.addParam<bool>("reversible", false, "true for reversible behavior");
  params.addCoupledVar("beta", "Remapped volume fraction of liquid (two-phase only)");
  params.addRequiredParam<MaterialPropertyName>("alpha", "Volume fraction");
  params.addRequiredCoupledVar("A", "Area");
  params.addRequiredCoupledVar("arhoA", "alpha*rho*A");
  params.addRequiredCoupledVar("arhouA", "alpha*rho*u*A");
  params.addCoupledVar("vel", "x-component of velocity");
  params.addRequiredParam<Real>("p_in", "The desired static pressure at the boundary");

  params.declareControllable("p_in");

  return params;
}

OneDEnergyStaticPressureLegacyBC::OneDEnergyStaticPressureLegacyBC(
    const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<OneDIntegratedBC>(parameters),
    _reversible(getParam<bool>("reversible")),
    _alpha(getMaterialProperty<Real>("alpha")),
    _dalpha_dbeta(isCoupled("beta") ? &getMaterialPropertyDerivativeTHM<Real>("alpha", "beta")
                                    : nullptr),
    _area(coupledValue("A")),
    _arhoA(coupledValue("arhoA")),
    _arhouA(coupledValue("arhouA")),
    _vel_old(_reversible ? coupledValueOld("vel") : _zero),
    _arhoA_var_number(coupled("arhoA")),
    _arhouA_var_number(coupled("arhouA")),
    _beta_var_num(isCoupled("beta") ? coupled("beta") : libMesh::invalid_uint),
    _p_in(getParam<Real>("p_in"))
{
}

bool
OneDEnergyStaticPressureLegacyBC::shouldApply()
{
  return !_reversible && THM::isOutlet(_vel_old[0], _normal);
}

Real
OneDEnergyStaticPressureLegacyBC::computeQpResidual()
{
  return _arhouA[_qp] / _arhoA[_qp] * (_u[_qp] + _alpha[_qp] * _area[_qp] * _p_in) * _normal *
         _test[_i][_qp];
}

Real
OneDEnergyStaticPressureLegacyBC::computeQpJacobian()
{
  return (_arhouA[_qp] / _arhoA[_qp]) * _normal * _phi[_j][_qp] * _test[_i][_qp];
}

Real
OneDEnergyStaticPressureLegacyBC::computeQpOffDiagJacobian(unsigned jvar)
{
  if (jvar == _beta_var_num)
  {
    return (*_dalpha_dbeta)[_qp] * _area[_qp] * _arhouA[_qp] / _arhoA[_qp] * _p_in * _normal *
           _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _arhoA_var_number)
  {
    return -_arhouA[_qp] / _arhoA[_qp] / _arhoA[_qp] *
           (_u[_qp] + _alpha[_qp] * _area[_qp] * _p_in) * _normal * _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _arhouA_var_number)
  {
    return 1. / _arhoA[_qp] * (_u[_qp] + _alpha[_qp] * _area[_qp] * _p_in) * _normal *
           _phi[_j][_qp] * _test[_i][_qp];
  }
  else
    return 0.;
}
