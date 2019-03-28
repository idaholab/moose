#include "EntropyViscosityCoefficientsMaterial.h"
#include "NormalizationParameter.h"

registerMooseObject("THMApp", EntropyViscosityCoefficientsMaterial);

template <>
InputParameters
validParams<EntropyViscosityCoefficientsMaterial>()
{
  InputParameters params = validParams<Material>();

  // Param for EVM
  params.addParam<Real>("Cmax", 0.5, "First order viscosity constant.");
  params.addParam<Real>("Cjump", 1.0, "Tuning coefficient for jumps in second-order viscosity.");
  params.addParam<Real>(
      "Centropy", 1.0, "Tuning coefficient for entropy residual in second-order viscosity.");
  params.addParam<bool>(
      "use_jump", true, "Use jumps instead of gradient for jump term in entropy viscosity");
  params.addParam<bool>(
      "use_first_order", false, "boolean to use the first-order visc. coefficients");
  params.addParam<bool>(
      "use_low_mach_fix",
      false,
      "boolean to use the low-Mach correction for the first-order visc. coefficients");
  params.addParam<bool>("use_parabolic_regularization",
                        false,
                        "boolean to use parabolic regularization: Pe = Re = M^2");
  // Coupled variables
  params.addRequiredCoupledVar("rho", "Density");
  params.addRequiredCoupledVar("rhou", "Momentum density");
  params.addRequiredCoupledVar("rhoE", "Total energy density");
  params.addRequiredCoupledVar("p", "Pressure of fluid");
  // Jump variables:
  params.addCoupledVar("jump_pressure", "variable storing the jump of the pressure.");
  params.addCoupledVar("jump_density", "variable storing the jump of the density.");

  params.addRequiredParam<MaterialPropertyName>(
      "direction", "The direction of the flow channel material property");
  params.addRequiredParam<MaterialPropertyName>("c", "Sound speed material property");

  // UserObjects
  params.addRequiredParam<UserObjectName>(
      "norm_param", "The name of the userobject used to compute the normalization parameter.");

  return params;
}

EntropyViscosityCoefficientsMaterial::EntropyViscosityCoefficientsMaterial(
    const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Material>(parameters),
    // Constant:
    _Cmax(getParam<Real>("Cmax")),
    _Cjump(getParam<Real>("Cjump")),
    _Centropy(getParam<Real>("Centropy")),
    // Param for EVM:
    _use_first_order(getParam<bool>("use_first_order")),
    _use_low_mach_fix(getParam<bool>("use_low_mach_fix")),
    _use_parabolic_regularization(getParam<bool>("use_parabolic_regularization")),
    _use_jump(getParam<bool>("use_jump")),
    // Density:
    _rho(coupledValue("rho")),
    _rho_dot(coupledDot("rho")),
    _grad_rho(coupledGradient("rho")),
    // Momentum:
    _rhou(coupledValue("rhou")),
    // Total energy:
    _rhoE(coupledValue("rhoE")),
    // Pressure:
    _press_dot(coupledDot("p")),
    _grad_press(coupledGradient("p")),
    // Jump variables:
    _jump_press(isCoupled("jump_pressure") ? coupledValue("jump_pressure") : _zero),
    _jump_dens(isCoupled("jump_density") ? coupledValue("jump_density") : _zero),
    _c(getMaterialProperty<Real>("c")),
    // Declare material properties: viscosity coefficients
    _kappa(declareProperty<Real>("evm:kappa")),

    _mu(declareProperty<Real>("evm:mu")),
    _dmu_drhoA(declarePropertyDerivativeTHM<Real>("evm:mu", "rho")),
    _dmu_drhouA(declarePropertyDerivativeTHM<Real>("evm:mu", "rhou")),
    _dmu_drhoEA(declarePropertyDerivativeTHM<Real>("evm:mu", "rhoE")),

    _visc_max(declareProperty<Real>("evm:visc_max")),

    _res(declareProperty<Real>("evm:residual")),

    _dir(getMaterialProperty<RealVectorValue>("direction")),

    _norm_param(getUserObject<NormalizationParameter>("norm_param"))
{
}

Real
EntropyViscosityCoefficientsMaterial::computeViscosity(
    Real h, Real residual, Real jump, Real rho, Real norm)
{
  Real viscosity = h * h * std::max(residual, jump);
  viscosity /= (rho * norm);
  return viscosity;
}

Real
EntropyViscosityCoefficientsMaterial::computeJump(
    Real press, Real press_grad, Real dens, Real dens_grad, Real norm)
{
  Real jump_max = std::max(press, norm * dens);
  Real jump_grad = std::max(std::fabs(press_grad), norm * std::fabs(dens_grad));
  return _use_jump ? jump_max : jump_grad;
}

void
EntropyViscosityCoefficientsMaterial::computeQpProperties()
{
  // Get the size of the 1D cell
  Real h = _current_elem->hmin();

  // Compute the velocity and mach number
  Real vel = _rhou[_qp] / _rho[_qp];
  Real speed = std::fabs(vel);
  Real M = speed / _c[_qp];
  // Compute the normalization parameter
  Real c2 = _c[_qp] * _c[_qp];
  Real norm = _norm_param.compute(c2, vel);

  // Compute the first order viscosity:
  _visc_max[_qp] = _Cmax * h * (speed + _c[_qp]);
  if (_use_low_mach_fix && (M < 1.0))
    _visc_max[_qp] *= M;

  // Compute the residual:
  Real residual = vel * _grad_press[_qp] * _dir[_qp];
  residual += _press_dot[_qp];
  residual -= c2 * vel * _grad_rho[_qp] * _dir[_qp];
  residual -= c2 * (_rho_dot[_qp]);
  residual = std::fabs(residual);
  _res[_qp] = residual;
  residual *= _Centropy;

  // Compute the jumps:
  // TODO: look at normalization
  Real jump = computeJump(_jump_press[_qp],
                          _grad_press[_qp] * _dir[_qp],
                          _jump_dens[_qp],
                          _grad_rho[_qp] * _dir[_qp],
                          norm);
  jump *= _Cjump * speed;

  // Compute the entropy viscosity coefficients: _mu_e and _kappa_e:
  Real mu_e = computeViscosity(h, residual, jump, _rho[_qp], norm);
  Real kappa_e;
  if (!_use_parabolic_regularization)
  {
    jump = computeJump(_jump_press[_qp],
                       _grad_press[_qp] * _dir[_qp],
                       _jump_dens[_qp],
                       _grad_rho[_qp] * _dir[_qp],
                       c2);
    jump *= _Cjump * speed;
    kappa_e = computeViscosity(h, residual, jump, _rho[_qp], c2);
  }
  else
  {
    kappa_e = mu_e;
  }

  // Choose between the first order viscosity and the entropy viscosity coefficients:
  if (_t_step <= 0 || _use_first_order)
  {
    _kappa[_qp] = _visc_max[_qp];
    _mu[_qp] = _kappa[_qp];
  }
  else
  {
    _kappa[_qp] = std::min(_visc_max[_qp], kappa_e);
    _mu[_qp] = std::min(_visc_max[_qp], mu_e);
  }
}
