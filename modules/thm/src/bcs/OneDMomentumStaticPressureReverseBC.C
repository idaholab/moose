#include "OneDMomentumStaticPressureReverseBC.h"
#include "SinglePhaseFluidProperties.h"
#include "THMApp.h"
#include "Numerics.h"

registerMooseObject("THMApp", OneDMomentumStaticPressureReverseBC);

InputParameters
OneDMomentumStaticPressureReverseBC::validParams()
{
  InputParameters params = OneDIntegratedBC::validParams();

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
    _vel(coupledValue("vel")),
    _vel_old(coupledValueOld("vel")),
    _area(coupledValue("A")),
    _arhoA(coupledValue("arhoA")),
    _temperature(coupledValue("T")),
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
  return (rho * _vel[_qp] * _vel[_qp] * _area[_qp] + _area[_qp] * _p) * _normal * _test[_i][_qp];
}

Real
OneDMomentumStaticPressureReverseBC::computeQpJacobian()
{
  Real rho = _spfp.rho_from_p_T(_p, _temperature[_qp]);
  return 2. * (rho / _arhoA[_qp]) * _vel[_qp] * _area[_qp] * _phi[_j][_qp] * _test[_i][_qp] *
         _normal;
}

Real
OneDMomentumStaticPressureReverseBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  Real rho = _spfp.rho_from_p_T(_p, _temperature[_qp]);

  if (jvar == _arhoA_varnum)
    return -2. * (rho / _arhoA[_qp]) * _vel[_qp] * _vel[_qp] * _area[_qp] * _phi[_j][_qp] *
           _test[_i][_qp] * _normal;
  else
    return 0.;
}
