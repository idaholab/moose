#include "OneDMomentumStaticPressureReverseBC.h"
#include "SinglePhaseFluidProperties.h"
#include "THMApp.h"
#include "Numerics.h"

registerMooseObject("THMApp", OneDMomentumStaticPressureReverseBC);

template <>
InputParameters
validParams<OneDMomentumStaticPressureReverseBC>()
{
  InputParameters params = validParams<OneDIntegratedBC>();

  params.addParam<bool>("is_liquid", true, "Does the phase correspond to liquid? (two-phase only)");
  params.addCoupledVar("alpha", 1.0, "Volume fraction");
  params.addCoupledVar("beta", "Remapped volume fraction of liquid (two-phase only)");
  params.addRequiredCoupledVar("vel", "");
  params.addRequiredCoupledVar("A", "Area");
  params.addRequiredCoupledVar("arhoA", "");
  params.addRequiredCoupledVar("T", "Temperature");

  params.addRequiredParam<Real>("p", "Static pressure at the boundary");
  params.addRequiredParam<UserObjectName>("fp", "The name of fluid properties user object to use.");

  params.declareControllable("p");

  return params;
}

OneDMomentumStaticPressureReverseBC::OneDMomentumStaticPressureReverseBC(
    const InputParameters & parameters)
  : OneDIntegratedBC(parameters),
    _is_liquid(getParam<bool>("is_liquid")),
    _sign(_is_liquid ? 1. : -1.),
    _alpha(coupledValue("alpha")),
    _vel(coupledValue("vel")),
    _vel_old(coupledValueOld("vel")),
    _area(coupledValue("A")),
    _arhoA(coupledValue("arhoA")),
    _temperature(coupledValue("T")),
    _beta_varnum(isCoupled("beta") ? coupled("beta") : libMesh::invalid_uint),
    _arhoA_varnum(coupled("arhoA")),
    _p(getParam<Real>("p")),
    _spfp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

bool
OneDMomentumStaticPressureReverseBC::shouldApply()
{
  return THM::isInlet(_vel_old[0], _normal);
}

Real
OneDMomentumStaticPressureReverseBC::computeQpResidual()
{
  Real rho = _spfp.rho_from_p_T(_p, _temperature[_qp]);
  return (_alpha[_qp] * rho * _vel[_qp] * _vel[_qp] * _area[_qp] + _alpha[_qp] * _area[_qp] * _p) *
         _normal * _test[_i][_qp];
}

Real
OneDMomentumStaticPressureReverseBC::computeQpJacobian()
{
  Real rho = _spfp.rho_from_p_T(_p, _temperature[_qp]);
  return 2. * (_alpha[_qp] * rho / _arhoA[_qp]) * _vel[_qp] * _area[_qp] * _phi[_j][_qp] *
         _test[_i][_qp] * _normal;
}

Real
OneDMomentumStaticPressureReverseBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  Real rho = _spfp.rho_from_p_T(_p, _temperature[_qp]);

  if (jvar == _beta_varnum)
    return _sign * (rho * _vel[_qp] * _vel[_qp] + _p) * _normal * _phi[_j][_qp] * _test[_i][_qp];
  else if (jvar == _arhoA_varnum)
    return -2. * (_alpha[_qp] * rho / _arhoA[_qp]) * _vel[_qp] * _vel[_qp] * _area[_qp] *
           _phi[_j][_qp] * _test[_i][_qp] * _normal;
  else
    return 0.;
}
