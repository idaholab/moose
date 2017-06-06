#include "FluidProperties3EqnMaterial.h"
#include "SinglePhaseFluidProperties.h"
#include "Numerics.h"

template <>
InputParameters
validParams<FluidProperties3EqnMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVar("area", "Cross-sectional area");
  params.addRequiredCoupledVar("rhoA", "Conserved density");
  params.addRequiredCoupledVar("rhouA", "Conserved momentum");
  params.addRequiredCoupledVar("rhoEA", "Conserved total energy");
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");
  return params;
}

FluidProperties3EqnMaterial::FluidProperties3EqnMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterfaceRelap<Material>(parameters),
    _area(coupledValue("area")),
    _rhoA(coupledValue("rhoA")),
    _rhouA(coupledValue("rhouA")),
    _rhoEA(coupledValue("rhoEA")),

    _rho(declareProperty<Real>("rho")),
    _drho_drhoA(declarePropertyDerivativeRelap<Real>("rho", "rhoA")),

    _v(declareProperty<Real>("v")),
    _dv_drhoA(declarePropertyDerivativeRelap<Real>("v", "rhoA")),

    _vel(declareProperty<Real>("vel")),
    _dvel_drhoA(declarePropertyDerivativeRelap<Real>("vel", "rhoA")),
    _dvel_drhouA(declarePropertyDerivativeRelap<Real>("vel", "rhouA")),

    _e(declareProperty<Real>("e")),
    _de_drhoA(declarePropertyDerivativeRelap<Real>("e", "rhoA")),
    _de_drhouA(declarePropertyDerivativeRelap<Real>("e", "rhouA")),
    _de_drhoEA(declarePropertyDerivativeRelap<Real>("e", "rhoEA")),

    _p(declareProperty<Real>("pressure")),
    _dp_drhoA(declarePropertyDerivativeRelap<Real>("pressure", "rhoA")),
    _dp_drhouA(declarePropertyDerivativeRelap<Real>("pressure", "rhouA")),
    _dp_drhoEA(declarePropertyDerivativeRelap<Real>("pressure", "rhoEA")),

    _T(declareProperty<Real>("temperature")),
    _dT_drhoA(declarePropertyDerivativeRelap<Real>("temperature", "rhoA")),
    _dT_drhouA(declarePropertyDerivativeRelap<Real>("temperature", "rhouA")),
    _dT_drhoEA(declarePropertyDerivativeRelap<Real>("temperature", "rhoEA")),

    _c(declareProperty<Real>("c")),
    _cp(declareProperty<Real>("cp")),
    _cv(declareProperty<Real>("cv")),
    _mu(declareProperty<Real>("mu")),
    _k(declareProperty<Real>("k")),

    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

void
FluidProperties3EqnMaterial::computeQpProperties()
{
  _rho[_qp] = _rhoA[_qp] / _area[_qp];
  _drho_drhoA[_qp] = 1.0 / _area[_qp];

  _v[_qp] = 1.0 / _rho[_qp];
  _dv_drhoA[_qp] = dv_darhoA(_area[_qp], _rhoA[_qp]);

  _vel[_qp] = _rhouA[_qp] / _rhoA[_qp];
  _dvel_drhoA[_qp] = -_rhouA[_qp] / (_rhoA[_qp] * _rhoA[_qp]);
  _dvel_drhouA[_qp] = 1.0 / _rhoA[_qp];

  _e[_qp] = (_rhoEA[_qp] - 0.5 * _rhouA[_qp] * _rhouA[_qp] / _rhoA[_qp]) / _rhoA[_qp];
  _de_drhoA[_qp] = de_darhoA(_rhoA[_qp], _rhouA[_qp], _rhoEA[_qp]);
  _de_drhouA[_qp] = de_darhouA(_rhoA[_qp], _rhouA[_qp]);
  _de_drhoEA[_qp] = de_darhoEA(_rhoA[_qp]);

  _p[_qp] = _fp.pressure(_v[_qp], _e[_qp]);
  _T[_qp] = _fp.temperature(_v[_qp], _e[_qp]);

  Real dp_dv, dp_de;
  Real dT_dv, dT_de;
  _fp.dp_duv(_v[_qp], _e[_qp], dp_dv, dp_de, dT_dv, dT_de);

  _dp_drhoA[_qp] = dp_dv * _dv_drhoA[_qp] + dp_de * _de_drhoA[_qp];
  _dp_drhouA[_qp] = dp_de * _de_drhouA[_qp];

  _dT_drhoA[_qp] = dT_dv * _dv_drhoA[_qp] + dT_de * _de_drhoA[_qp];
  _dT_drhouA[_qp] = dT_de * _de_drhouA[_qp];

  _dp_drhoEA[_qp] = dp_de * _de_drhoEA[_qp];
  _dT_drhoEA[_qp] = dT_de * _de_drhoEA[_qp];

  _c[_qp] = _fp.c(_v[_qp], _e[_qp]);
  _cp[_qp] = _fp.cp(_v[_qp], _e[_qp]);
  _cv[_qp] = _fp.cv(_v[_qp], _e[_qp]);
  _mu[_qp] = _fp.mu(_v[_qp], _e[_qp]);
  _k[_qp] = _fp.k(_v[_qp], _e[_qp]);
}
