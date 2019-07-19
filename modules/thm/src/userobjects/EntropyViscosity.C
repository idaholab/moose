#include "EntropyViscosity.h"
#include "FlowModel.h"

registerMooseObject("THMApp", EntropyViscosity);

template <>
InputParameters
validParams<EntropyViscosity>()
{
  InputParameters params = validParams<StabilizationSettings>();
  // Single-phase parameters
  params.addParam<Real>(
      "Cmax",
      0.5,
      "scaling of the first order viscosity used in the Entropy Viscosity Method (EVM).");
  params.addParam<Real>(
      "Cjump", 1.0, "coefficient for jumps used in the Entropy Viscosity Method (EVM).");
  params.addParam<Real>(
      "Centropy",
      1.0,
      "residual constant for second order viscosity in the Entropy Viscosity Method (EVM).");
  params.addParam<bool>("use_first_order", false, "Use first order visc. coefficients");
  params.addParam<bool>(
      "use_parabolic_regularization", false, "Use parabolic regularization for the EVM");
  params.addParam<bool>(
      "use_jump", true, "Use jumps instead of gradient for jump term in entropy viscosity");
  MooseEnum mrf("Mach Tanh Sin Shock", "Mach");
  params.addParam<MooseEnum>("mach_regime_function",
                             mrf,
                             "Function used to classify whether a flow is in the low mach "
                             "regime. This is used in the definition of the visc. coeff.");
  params.addParam<Real>("M_thres", 0.3, "coefficient normalization parameter.");
  params.addParam<Real>("a", 0.05, "coefficient normalization parameter.");

  // Two-phase parameters
  // Volume fraction
  params.addParam<Real>("Cmax_vf",
                        0.5,
                        "scaling of the first order viscosity used in the Entropy "
                        "Viscosity Method (EVM) for the volume fraction equation.");
  params.addParam<Real>("Cjump_vf",
                        1.,
                        "coefficient for jumps used in the Entropy Viscosity "
                        "Method (EVM) for the volume fraction equation.");
  params.addParam<Real>("Centropy_vf",
                        1.0,
                        "residual constant for second order viscosity in the "
                        "Entropy Viscosity Method (EVM) for the volume "
                        "fraction equation.");
  params.addParam<bool>(
      "use_first_order_vf", false, "Use first order visc. coefficients only in the vf equ");
  params.addParam<bool>("use_jump_vf",
                        true,
                        "Use jumps instead of gradient for jump term in "
                        "entropy viscosity for the volume fraction equation");
  // Liquid
  params.addParam<Real>("Cmax_liquid",
                        0.5,
                        "scaling of the first order viscosity used in the "
                        "Entropy Viscosity Method (EVM) for the liquid phase.");
  params.addParam<Real>(
      "Cjump_liquid",
      1.,
      "coefficient for jumps used in the Entropy Viscosity Method (EVM) for the liquid phase.");
  params.addParam<Real>("Centropy_liquid",
                        1.0,
                        "residual constant for second order viscosity in "
                        "the Entropy Viscosity Method (EVM) for the liquid "
                        "phase.");
  params.addParam<bool>("use_first_order_liquid", false, "Use first order visc. coefficients");
  params.addParam<bool>(
      "use_parabolic_regularization_liquid", false, "Use parabolic regularization for the EVM");
  params.addParam<bool>(
      "use_jump_liquid",
      true,
      "Use jumps instead of gradient for jump term in entropy viscosity for the liquid phase");
  params.addParam<MooseEnum>("mach_regime_function_liquid",
                             mrf,
                             "Function used to classify "
                             "whether a flow is in the low "
                             "mach regime. This is used in the "
                             "definition of the visc. coeff.");
  params.addParam<Real>("M_thres_liquid", 0.3, "coefficient normalization parameter.");
  params.addParam<Real>("a_liquid", 0.05, "coefficient normalization parameter.");
  // Vapor
  params.addParam<Real>("Cmax_vapor",
                        0.5,
                        "scaling of the first order viscosity used in the "
                        "Entropy Viscosity Method (EVM) for the vapor phase.");
  params.addParam<Real>(
      "Cjump_vapor",
      1.,
      "coefficient for jumps used in the Entropy Viscosity Method (EVM) for the vapor phase.");
  params.addParam<Real>("Centropy_vapor",
                        1.0,
                        "residual constant for second order viscosity in "
                        "the Entropy Viscosity Method (EVM) for the vapor "
                        "phase.");
  params.addParam<bool>("use_first_order_vapor", false, "Use first order visc. coefficients");
  params.addParam<bool>(
      "use_parabolic_regularization_vapor", false, "Use parabolic regularization for the EVM");
  params.addParam<bool>(
      "use_jump_vapor",
      true,
      "Use jumps instead of gradient for jump term in entropy viscosity for the vapor phase");
  params.addParam<MooseEnum>("mach_regime_function_vapor",
                             mrf,
                             "Function used to classify whether "
                             "a flow is in the low mach regime. "
                             "This is used in the definition of "
                             "the visc. coeff.");
  params.addParam<Real>("M_thres_vapor", 0.3, "coefficient normalization parameter.");
  params.addParam<Real>("a_vapor", 0.05, "coefficient normalization parameter.");

  // Parameters shared between single-phase and two-phase
  params.addParam<bool>("use_low_mach_fix", false, "Use low-Mach fix");
  return params;
}

EntropyViscosity::EntropyViscosity(const InputParameters & parameters)
  : StabilizationSettings(parameters)
{
  mooseError("Entropy viscosity method is no longer available. Update your input file to use rDG.");
}

void
EntropyViscosity::addVariables(FlowModel &, SubdomainID) const
{
}

void
EntropyViscosity::initMooseObjects(FlowModel &)
{
}

void
EntropyViscosity::addMooseObjects(FlowModel &, InputParameters &) const
{
}
