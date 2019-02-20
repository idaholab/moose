#include "OneDMomentumStaticPressureBC.h"
#include "Numerics.h"

registerMooseObject("THMApp", OneDMomentumStaticPressureBC);

template <>
InputParameters
validParams<OneDMomentumStaticPressureBC>()
{
  InputParameters params = validParams<OneDIntegratedBC>();
  params.addParam<bool>("reversible", false, "true for reversible behavior");
  params.addCoupledVar("beta", "Remapped volume fraction of liquid (two-phase only)");
  params.addRequiredCoupledVar("rhoA", "Conserved density of the phase");
  params.addRequiredCoupledVar("rhoEA", "Conserved total energy of the phase");
  params.addRequiredCoupledVar("A", "Area");
  params.addRequiredParam<MaterialPropertyName>("alpha", "Volume fraction");
  params.addCoupledVar("vel", "x-component of velocity, needed when reversible is true");
  params.addRequiredParam<Real>("p_in", "Specified pressure");

  params.declareControllable("p_in");

  return params;
}

OneDMomentumStaticPressureBC::OneDMomentumStaticPressureBC(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<OneDIntegratedBC>(parameters),
    _reversible(getParam<bool>("reversible")),
    _rhoA_var_number(coupled("rhoA")),
    _rhoEA_var_number(coupled("rhoEA")),
    _beta_var_num(isCoupled("beta") ? coupled("beta") : libMesh::invalid_uint),
    _alpha(getMaterialProperty<Real>("alpha")),
    _dalpha_dbeta(isCoupled("beta") ? &getMaterialPropertyDerivativeTHM<Real>("alpha", "beta")
                                    : nullptr),
    _area(coupledValue("A")),
    _rhoA(coupledValue("rhoA")),
    _vel_old(_reversible ? coupledValueOld("vel") : _zero),
    _p_in(getParam<Real>("p_in"))
{
}

bool
OneDMomentumStaticPressureBC::shouldApply()
{
  return !_reversible || THM::isOutlet(_vel_old[0], _normal);
}

Real
OneDMomentumStaticPressureBC::computeQpResidual()
{
  return (_u[_qp] * _u[_qp] / _rhoA[_qp] + _alpha[_qp] * _area[_qp] * _p_in) * _normal *
         _test[_i][_qp];
}

Real
OneDMomentumStaticPressureBC::computeQpJacobian()
{
  return (2. * _u[_qp] / _rhoA[_qp]) * _normal * _phi[_j][_qp] * _test[_i][_qp];
}

Real
OneDMomentumStaticPressureBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _rhoA_var_number)
  {
    Real vel = _u[_qp] / _rhoA[_qp];
    return -(vel * vel) * _normal * _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _beta_var_num)
  {
    return (*_dalpha_dbeta)[_qp] * _area[_qp] * _p_in * _normal * _phi[_j][_qp] * _test[_i][_qp];
  }
  else
    return 0.;
}
