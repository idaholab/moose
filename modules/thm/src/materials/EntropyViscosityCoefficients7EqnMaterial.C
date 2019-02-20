#include "EntropyViscosityCoefficients7EqnMaterial.h"
#include "NormalizationParameter.h"

registerMooseObject("THMApp", EntropyViscosityCoefficients7EqnMaterial);

template <>
InputParameters
validParams<EntropyViscosityCoefficients7EqnMaterial>()
{
  InputParameters params = validParams<Material>();
  // Param for EVM
  params.addParam<Real>("Cmax", 0.5, "First order viscosity constant.");
  params.addParam<Real>("Cjump", 1.0, "Constant for second order visc. coeff. mu_e and kappa_e.");
  params.addParam<Real>(
      "Centropy", 1.0, "Tuning coefficient for entropy residual in second-order viscosity.");
  params.addParam<Real>(
      "Cmax_vf", 0.5, "First order viscosity constant for volume fraction equation.");
  params.addParam<Real>("Cjump_vf", 1.0, "Constant for second order visc. coeff. beta_e.");
  params.addParam<Real>("Centropy_vf",
                        1.0,
                        "Tuning coefficient for entropy residual in "
                        "second-order viscosity for volume fraction equation.");

  params.addParam<bool>(
      "use_jump", true, "Use jumps instead of gradient for jump term in entropy viscosity");
  params.addParam<bool>("use_jump_vf",
                        true,
                        "Use jumps instead of gradient for jump term in "
                        "entropy viscosity in volume fraction equation");
  params.addParam<bool>(
      "use_low_mach_fix",
      false,
      "boolean to use the low-Mach correction for the first-order visc. coefficients");
  params.addParam<bool>(
      "use_first_order", false, "boolean to use the first-order visc. coefficients");
  params.addParam<bool>(
      "use_first_order_vf",
      false,
      "boolean to use the first-order visc. coefficients for the volume fraciton equation");

  params.addParam<bool>(
      "use_parabolic_regularization", false, "use parabolic regularization: Pe = Re = M^2");
  params.addRequiredParam<bool>("is_liquid", "choose the type of phase");
  // Coupled variables
  params.addRequiredCoupledVar("rho", "Density");
  params.addRequiredCoupledVar("rhou", "Momentum density");
  params.addRequiredCoupledVar("rhoE", "Total energy density");
  params.addRequiredCoupledVar("p", "Pressure of fluid");
  params.addRequiredCoupledVar("alpha", "Volume fraction");
  // Jump variables:
  params.addCoupledVar("jump_pressure", 0.0, "variable storing the jump of the pressure.");
  params.addCoupledVar("jump_density", 0.0, "variable storing the jump of the density.");
  params.addCoupledVar("jump_alpha", 0.0, "variable storing the jump of the void-fraction.");
  // UserObjects
  params.addRequiredParam<UserObjectName>(
      "norm_param", "The name of the userobject used to compute the normalization parameter.");
  // Pps
  params.addRequiredParam<PostprocessorName>("vf_PPS_name", "pps for volume fraction");
  params.addRequiredParam<MaterialPropertyName>("direction",
                                                "The direction of the pipe material property");
  params.addRequiredParam<MaterialPropertyName>(
      "c", "The name of material property that hold sound speed for this phase");
  params.addRequiredParam<MaterialPropertyName>("vel_int",
                                                "Interfacial velocity material property");
  params.addRequiredParam<MaterialPropertyName>("evm_visc_max",
                                                "Max viscosity coefficient material property");
  params.addRequiredParam<MaterialPropertyName>("evm_kappa",
                                                "Kappa viscosity coefficient material property");
  params.addRequiredParam<MaterialPropertyName>("evm_mu",
                                                "Mu viscosity coefficient material property");
  params.addRequiredParam<MaterialPropertyName>("evm_beta",
                                                "Beta viscosity coefficient material property");
  params.addRequiredParam<MaterialPropertyName>("evm_beta_max",
                                                "Max beta viscosity coefficient material property");
  return params;
}

EntropyViscosityCoefficients7EqnMaterial::EntropyViscosityCoefficients7EqnMaterial(
    const InputParameters & parameters)
  : Material(parameters),
    // Param for EVM
    _Cmax(getParam<Real>("Cmax")),
    _Cjump(getParam<Real>("Cjump")),
    _Centropy(getParam<Real>("Centropy")),
    _Cmax_vf(getParam<Real>("Cmax_vf")),
    _Cjump_vf(getParam<Real>("Cjump_vf")),
    _Centropy_vf(getParam<Real>("Centropy_vf")),
    _use_jump(getParam<bool>("use_jump")),
    _use_jump_vf(getParam<bool>("use_jump_vf")),
    _use_first_order(getParam<bool>("use_first_order")),
    _use_first_order_vf(getParam<bool>("use_first_order_vf")),
    _use_low_mach_fix(getParam<bool>("use_low_mach_fix")),
    _use_parabolic_regularization(getParam<bool>("use_parabolic_regularization")),
    _is_liquid(getParam<bool>("is_liquid")),
    // Density:
    _rho(coupledValue("rho")),
    _rho_old(coupledValueOld("rho")),
    _rho_dot(coupledDot("rho")),
    _grad_rho(coupledGradient("rho")),
    // Momentum:
    _rhou(coupledValue("rhou")),
    _rhou_old(coupledValueOld("rhou")),
    // Total energy:
    _rhoE(coupledValue("rhoE")),
    _rhoE_old(coupledValueOld("rhoE")),
    // Pressure:
    _press(coupledValue("p")),
    _press_dot(coupledDot("p")),
    _grad_press(coupledGradient("p")),
    // Phase void fraction:
    _alpha(coupledValue("alpha")),
    _alpha_dot(coupledDot("alpha")),
    _grad_alpha(coupledGradient("alpha")),
    // Jump variables:
    _jump_press(coupledValue("jump_pressure")),
    _jump_dens(coupledValue("jump_density")),
    _jump_alpha(coupledValue("jump_alpha")),
    //
    _c(getMaterialProperty<Real>("c")),
    // Declare material properties: viscosity coefficients
    _visc_max(declareProperty<Real>(getParam<MaterialPropertyName>("evm_visc_max"))),
    _kappa(declareProperty<Real>(getParam<MaterialPropertyName>("evm_kappa"))),
    _mu(declareProperty<Real>(getParam<MaterialPropertyName>("evm_mu"))),
    _beta_max(declareProperty<Real>(getParam<MaterialPropertyName>("evm_beta_max"))),
    _beta(declareProperty<Real>(getParam<MaterialPropertyName>("evm_beta"))),
    // Get material:
    _vI(getMaterialProperty<Real>("vel_int")),
    // Userobjects
    _norm_param(getUserObject<NormalizationParameter>("norm_param")),
    // Pps
    _vf_pps(getPostprocessorValue("vf_PPS_name")),
    _dir(getMaterialProperty<RealVectorValue>("direction"))
{
}

void
EntropyViscosityCoefficients7EqnMaterial::computeQpProperties()
{
  // Get the size of the 1D cell
  Real h = _current_elem->hmin();

  // Compute the velocity and mach number
  Real vel = _rhou[_qp] / _rho[_qp];
  Real speed = std::fabs(vel);
  Real M = speed / _c[_qp];

  /// Compute viscosity coefficients for volume fraction equation (beta_e and beta_max): ///
  // Compute first-order viscosity: beta_max
  const Real vI = _vI[_qp];
  _beta_max[_qp] = _Cmax_vf * h * std::fabs(vI);
  if (_use_low_mach_fix && (M < 1.0))
    _beta_max[_qp] *= M;

  // Compute the void-fraction entropy residual:
  Real residual = _Centropy_vf * _alpha_dot[_qp] + vI * _grad_alpha[_qp] * _dir[_qp];

  // Compute the normalization parameter:
  Real norm = _vf_pps + 1e-10;

  // Compute the entropy viscosity coefficient: beta_e
  Real jump_max = _jump_alpha[_qp];
  Real jump_grad = std::fabs(_grad_alpha[_qp] * _dir[_qp]);
  Real jump = _use_jump_vf ? jump_max : jump_grad;
  jump *= _Cjump_vf * std::fabs(vI);
  Real beta_e = h * h * std::max(std::fabs(residual), jump) / norm;

  if (_t_step <= 1 || _use_first_order_vf)
    _beta[_qp] = _beta_max[_qp];
  else
    _beta[_qp] = std::min(_beta_max[_qp], beta_e);

  /// Compute viscosity coefficients for continuity, momentum and energy equations: ///

  // Compute the first order viscosity: mu_max and beta_max
  _visc_max[_qp] = _Cmax * h * (std::fabs(vel) + _c[_qp]);
  if (_use_low_mach_fix && (M < 1.0))
    _visc_max[_qp] *= M;

  // Compute the normalization parameter:
  Real c2 = _c[_qp] * _c[_qp];
  norm = _norm_param.compute(c2, vel);

  // Compute the pressure and density residual:
  residual = 0.;
  residual = vel * _grad_press[_qp] * _dir[_qp];
  residual += _press_dot[_qp];
  residual -= c2 * vel * _grad_rho[_qp] * _dir[_qp];
  residual -= c2 * _rho_dot[_qp];
  residual *= _Centropy;

  // Compute the jumps:
  jump_max = std::max(_jump_press[_qp], norm * _jump_dens[_qp]);
  jump_grad = std::max(std::fabs(_grad_press[_qp] * _dir[_qp]),
                       norm * std::fabs(_grad_rho[_qp] * _dir[_qp]));
  jump = _use_jump ? jump_max : jump_grad;
  jump *= _Cjump * std::fabs(vel);

  // Compute the entropy viscosity coefficients: _mu_e and _kappa_e:
  Real kappa_e, mu_e;
  if (!_use_parabolic_regularization)
  {
    mu_e = h * h * std::max(std::fabs(residual), jump) / (_rho[_qp] * norm);
    jump_max = std::max(_jump_press[_qp], 0.5 * c2 * _jump_dens[_qp]);
    jump_grad = std::max(std::fabs(_grad_press[_qp] * _dir[_qp]),
                         0.5 * c2 * std::fabs(_grad_rho[_qp] * _dir[_qp]));
    jump = _use_jump ? jump_max : jump_grad;
    jump *= _Cjump * std::fabs(vel);
    kappa_e = h * h * std::max(std::fabs(residual), jump) / (0.5 * _rho[_qp] * c2);
  }
  else
  {
    mu_e = h * h * std::max(std::fabs(residual), jump) / (_rho[_qp] * norm);
    kappa_e = mu_e;
  }

  // Choose between first-order and second-order viscosity coefficients:
  if (_t_step <= 1 || _use_first_order)
  {
    _kappa[_qp] = _visc_max[_qp];
    _mu[_qp] = _visc_max[_qp];
  }
  else
  {
    _kappa[_qp] = std::min(_visc_max[_qp], kappa_e);
    _mu[_qp] = std::min(_visc_max[_qp], mu_e);
  }
}
