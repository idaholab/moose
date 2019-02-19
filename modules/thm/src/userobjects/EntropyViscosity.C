#include "EntropyViscosity.h"
#include "FlowModel.h"
#include "Simulation.h"
#include "Component.h"
#include "InputParameterLogic.h"
#include "FlowModelSinglePhase.h"
#include "FlowModelTwoPhase.h"
#include "MooseVariable.h"

// 1-phase coefficients
const std::string EntropyViscosity::MAX_VISCOSITY = "evm:visc_max";
const std::string EntropyViscosity::KAPPA = "evm:kappa";
const std::string EntropyViscosity::MU = "evm:mu";
const std::string EntropyViscosity::RESIDUAL = "evm:residual";

// 2-phase coefficients
const std::string EntropyViscosity::MAX_VISCOSITY_LIQUID = "evm:visc_max_liquid";
const std::string EntropyViscosity::MAX_VISCOSITY_VAPOR = "evm:visc_max_vapor";
const std::string EntropyViscosity::KAPPA_LIQUID = "evm:kappa_liquid";
const std::string EntropyViscosity::KAPPA_VAPOR = "evm:kappa_vapor";
const std::string EntropyViscosity::MU_LIQUID = "evm:mu_liquid";
const std::string EntropyViscosity::MU_VAPOR = "evm:mu_vapor";
const std::string EntropyViscosity::BETA_LIQUID = "evm:beta";
const std::string EntropyViscosity::BETA_VAPOR = "evm:beta_zero";
const std::string EntropyViscosity::MAX_BETA_LIQUID = "evm:beta_max";
const std::string EntropyViscosity::MAX_BETA_VAPOR = "evm:beta_zero";

const std::string EntropyViscosity::JUMP_PRESSURE = "jump_pressure";
const std::string EntropyViscosity::JUMP_RHO = "jump_rho";

const std::string EntropyViscosity::JUMP_PRESSURE_LIQUID = "jump_pressure_liquid";
const std::string EntropyViscosity::JUMP_RHO_LIQUID = "jump_rho_liquid";
const std::string EntropyViscosity::JUMP_PRESSURE_VAPOR = "jump_pressure_vapor";
const std::string EntropyViscosity::JUMP_RHO_VAPOR = "jump_rho_vapor";
const std::string EntropyViscosity::JUMP_ALPHA = "jump_alpha";

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
  : StabilizationSettings(parameters),
    // single phase
    _Cmax(getParam<Real>("Cmax")),
    _Cjump(getParam<Real>("Cjump")),
    _Centropy(getParam<Real>("Centropy")),
    _use_first_order(getParam<bool>("use_first_order")),
    _use_parabolic_regularization(getParam<bool>("use_parabolic_regularization")),
    _use_jump(getParam<bool>("use_jump")),
    _mach_regime_function(getParam<MooseEnum>("mach_regime_function")),
    _M_thres(getParam<Real>("M_thres")),
    _a(getParam<Real>("a")),
    // two phase
    _Cmax_vf(getParam<Real>("Cmax_vf")),
    _Cjump_vf(getParam<Real>("Cjump_vf")),
    _Centropy_vf(getParam<Real>("Centropy_vf")),
    _use_first_order_vf(getParam<bool>("use_first_order_vf")),
    _use_jump_vf(getParam<bool>("use_jump_vf")),
    // Liquid
    _Cmax_liquid(getParam<Real>("Cmax_liquid")),
    _Cjump_liquid(getParam<Real>("Cjump_liquid")),
    _Centropy_liquid(getParam<Real>("Centropy_liquid")),
    _use_first_order_liquid(getParam<bool>("use_first_order_liquid")),
    _use_parabolic_regularization_liquid(getParam<bool>("use_parabolic_regularization_liquid")),
    _use_jump_liquid(getParam<bool>("use_jump_liquid")),
    _mach_regime_function_liquid(getParam<MooseEnum>("mach_regime_function_liquid")),
    _M_thres_liquid(getParam<Real>("M_thres_liquid")),
    _a_liquid(getParam<Real>("a_liquid")),
    // Vapor
    _Cmax_vapor(getParam<Real>("Cmax_vapor")),
    _Cjump_vapor(getParam<Real>("Cjump_vapor")),
    _Centropy_vapor(getParam<Real>("Centropy_vapor")),
    _use_first_order_vapor(getParam<bool>("use_first_order_vapor")),
    _use_parabolic_regularization_vapor(getParam<bool>("use_parabolic_regularization_vapor")),
    _use_jump_vapor(getParam<bool>("use_jump_vapor")),
    _mach_regime_function_vapor(getParam<MooseEnum>("mach_regime_function_vapor")),
    _M_thres_vapor(getParam<Real>("M_thres_vapor")),
    _a_vapor(getParam<Real>("a_vapor")),

    _use_low_mach_fix(getParam<bool>("use_low_mach_fix"))
{
}

void
EntropyViscosity::addVariables(FlowModel & fm, unsigned int subdomain_id) const
{
  if (dynamic_cast<FlowModelSinglePhase *>(&fm) != NULL)
  {
    if (_use_jump)
    {
      _m_sim.addVariable(false, JUMP_PRESSURE, FEType(CONSTANT, MONOMIAL), subdomain_id);
      _m_sim.addVariable(false, JUMP_RHO, FEType(CONSTANT, MONOMIAL), subdomain_id);
    }
  }
  else if (dynamic_cast<FlowModelTwoPhase *>(&fm) != NULL)
  {
    if (_use_jump_liquid)
    {
      _m_sim.addVariable(false, JUMP_PRESSURE_LIQUID, FEType(CONSTANT, MONOMIAL), subdomain_id);
      _m_sim.addVariable(false, JUMP_RHO_LIQUID, FEType(CONSTANT, MONOMIAL), subdomain_id);
    }
    if (_use_jump_vapor)
    {
      _m_sim.addVariable(false, JUMP_PRESSURE_VAPOR, FEType(CONSTANT, MONOMIAL), subdomain_id);
      _m_sim.addVariable(false, JUMP_RHO_VAPOR, FEType(CONSTANT, MONOMIAL), subdomain_id);
    }
    if (_use_jump_vf)
      _m_sim.addVariable(false, JUMP_ALPHA, FEType(CONSTANT, MONOMIAL), subdomain_id);
  }
}

void
EntropyViscosity::initMooseObjects(FlowModel & fm)
{
  bool isTwoPhase = dynamic_cast<FlowModelTwoPhase *>(&fm) != NULL;
  getOneOrTwoPhaseParameters<Real>(
      isTwoPhase, "Cmax", {"Cmax_vf", "Cmax_liquid", "Cmax_vapor"}, *this);
  getOneOrTwoPhaseParameters<Real>(
      isTwoPhase, "Cjump", {"Cjump_vf", "Cjump_liquid", "Cjump_vapor"}, *this);
  getOneOrTwoPhaseParameters<Real>(
      isTwoPhase, "Centropy", {"Centropy_vf", "Centropy_liquid", "Centropy_vapor"}, *this);
  getOneOrTwoPhaseParameters<bool>(
      isTwoPhase,
      "use_first_order",
      {"use_first_order_vf", "use_first_order_liquid", "use_first_order_vapor"},
      *this);
  getOneOrTwoPhaseParameters<bool>(
      isTwoPhase, "use_jump", {"use_jump_vf", "use_jump_liquid", "use_jump_vapor"}, *this);
  getOneOrTwoPhaseParameters<bool>(
      isTwoPhase,
      "use_parabolic_regularization",
      {"use_parabolic_regularization_liquid", "use_parabolic_regularization_vapor"},
      *this);
  getOneOrTwoPhaseParameters<MooseEnum>(
      isTwoPhase,
      "mach_regime_function",
      {"mach_regime_function_liquid", "mach_regime_function_vapor"},
      *this);
  getOneOrTwoPhaseParameters<Real>(
      isTwoPhase, "M_thres", {"M_thres_liquid", "M_thres_vapor"}, *this);
  getOneOrTwoPhaseParameters<Real>(isTwoPhase, "a", {"a_liquid", "a_vapor"}, *this);
}

void
EntropyViscosity::addMooseObjects(FlowModel & fm, InputParameters & pars) const
{
  if (dynamic_cast<FlowModelSinglePhase *>(&fm) != NULL)
    setup1Phase(dynamic_cast<FlowModelSinglePhase &>(fm), pars);
  else if (dynamic_cast<FlowModelTwoPhase *>(&fm) != NULL)
    setup2Phase(dynamic_cast<FlowModelTwoPhase &>(fm), pars);
}

void
EntropyViscosity::setup1Phase(FlowModelSinglePhase & /*fm*/, InputParameters & pars) const
{
  const auto & blocks = pars.get<std::vector<SubdomainName>>("block");
  Component * comp = pars.getCheckedPointerParam<Component *>("component");
  std::string comp_name = comp->name();

  // coupling vectors
  std::vector<VariableName> cv_rhoA(1, FlowModelSinglePhase::RHOA);
  std::vector<VariableName> cv_rhouA(1, FlowModelSinglePhase::RHOUA);
  std::vector<VariableName> cv_rhoEA(1, FlowModelSinglePhase::RHOEA);
  std::vector<VariableName> cv_rho(1, FlowModelSinglePhase::DENSITY);
  std::vector<VariableName> cv_rhou(1, FlowModelSinglePhase::MOMENTUM_DENSITY);
  std::vector<VariableName> cv_rhoE(1, FlowModelSinglePhase::TOTAL_ENERGY_DENSITY);
  std::vector<VariableName> cv_vel(1, FlowModelSinglePhase::VELOCITY);
  std::vector<VariableName> cv_area(1, FlowModel::AREA);
  std::vector<VariableName> cv_pressure(1, FlowModelSinglePhase::PRESSURE);
  std::vector<VariableName> cv_e(1, FlowModelSinglePhase::SPECIFIC_INTERNAL_ENERGY);
  std::vector<VariableName> cv_jump_press(1, JUMP_PRESSURE);
  std::vector<VariableName> cv_jump_dens(1, JUMP_RHO);

  // output
  std::vector<OutputName> outputs = _m_sim.getOutputsVector("none");

  // execute
  ExecFlagEnum execute_options(MooseUtils::getDefaultExecFlagEnum());
  execute_options = {EXEC_TIMESTEP_BEGIN, EXEC_INITIAL};

  // Velocity PPS: average value of the velocity.
  std::string avg_vel_pps_name = Component::genName(comp_name, "average_velocity");
  {
    std::string class_name = "ElementAverageValue";
    InputParameters params = _m_factory.getValidParams(class_name);
    params.set<std::vector<VariableName>>("variable") =
        std::vector<VariableName>(1, FlowModelSinglePhase::VELOCITY);
    params.set<std::vector<OutputName>>("outputs") = outputs;
    params.set<ExecFlagEnum>("execute_on") = execute_options;
    params.set<std::vector<SubdomainName>>("block") = blocks;
    _m_sim.addPostprocessor(class_name, avg_vel_pps_name, params);
  }

  // Jumps
  if (_use_jump)
  {
    // Jump of pressure gradient.
    {
      InputParameters params = _m_factory.getValidParams("JumpGradientInterface");
      params.set<std::vector<VariableName>>("variable") = cv_pressure;
      params.set<ExecFlagEnum>("execute_on") = execute_options;
      params.set<std::string>("jump") = "jump_pressure";
      params.set<std::vector<SubdomainName>>("block") = blocks;
      _m_sim.addUserObject(
          "JumpGradientInterface", Component::genName(comp_name, "jump_pressure"), params);
    }
    // Jump of density gradient.
    {
      InputParameters params = _m_factory.getValidParams("JumpGradientInterface");
      params.set<std::vector<VariableName>>("variable") = cv_rho;
      params.set<ExecFlagEnum>("execute_on") = execute_options;
      params.set<std::string>("jump") = "jump_rho";
      params.set<std::vector<SubdomainName>>("block") = blocks;
      _m_sim.addUserObject(
          "JumpGradientInterface", Component::genName(comp_name, "jump_rho"), params);
    }
  }

  // Normalization parameter
  std::string norm_param_name = Component::genName(comp_name, "norm_param");
  {
    std::string class_name = "NormalizationParameter";
    InputParameters params = _m_factory.getValidParams(class_name);
    params.set<ExecFlagEnum>("execute_on") = execute_options;
    params.set<Real>("M_threshold") = _M_thres;
    params.set<Real>("a") = _a;
    params.set<PostprocessorName>("avg_vel") = avg_vel_pps_name;
    params.set<MooseEnum>("funct_type") = _mach_regime_function;
    _m_sim.addUserObject(class_name, norm_param_name, params);
  }

  // Viscosity coefficients
  {
    std::string class_name = "EntropyViscosityCoefficientsMaterial";
    InputParameters params = _m_factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = blocks;
    params.set<bool>("use_first_order") = _use_first_order;
    params.set<bool>("use_low_mach_fix") = _use_low_mach_fix;
    params.set<Real>("Cmax") = _Cmax;
    params.set<Real>("Cjump") = _Cjump;
    params.set<Real>("Centropy") = _Centropy;
    params.set<bool>("use_parabolic_regularization") = _use_parabolic_regularization;
    params.set<bool>("use_jump") = _use_jump;

    params.set<std::vector<VariableName>>("rho") = cv_rho;
    params.set<std::vector<VariableName>>("rhou") = cv_rhou;
    params.set<std::vector<VariableName>>("rhoE") = cv_rhoE;
    params.set<std::vector<VariableName>>("p") = cv_pressure;
    if (_use_jump)
    {
      params.set<std::vector<VariableName>>("jump_pressure") = cv_jump_press;
      params.set<std::vector<VariableName>>("jump_density") = cv_jump_dens;
    }
    params.set<UserObjectName>("norm_param") = norm_param_name;

    params.set<MaterialPropertyName>("c") = FlowModelSinglePhase::SOUND_SPEED;
    params.set<MaterialPropertyName>("direction") = FlowModel::DIRECTION;
    _m_sim.addMaterial(class_name, Component::genName(comp_name, "ent_visc_vars"), params);
  }

  // Add viscous fluxes to the equations
  std::vector<NonlinearVariableName> var_name(3, "");
  var_name[0] = FlowModelSinglePhase::RHOA;
  var_name[1] = FlowModelSinglePhase::RHOUA;
  var_name[2] = FlowModelSinglePhase::RHOEA;

  std::vector<std::string> eqn_name(3, "");
  eqn_name[0] = "CONTINUITY";
  eqn_name[1] = "MOMENTUM";
  eqn_name[2] = "ENERGY";

  for (unsigned int i = 0; i < eqn_name.size(); i++)
  {
    std::string class_name = "OneDEntropyMinimumArtificialDissipation";
    InputParameters params = _m_factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = var_name[i];
    params.set<std::vector<SubdomainName>>("block") = blocks;
    params.set<MooseEnum>("eqn_name") = FlowModel::getFlowEquationType(eqn_name[i]);
    params.set<std::vector<VariableName>>("rhoA") = cv_rhoA;
    params.set<std::vector<VariableName>>("rhouA") = cv_rhouA;
    params.set<std::vector<VariableName>>("rhoEA") = cv_rhoEA;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("rho") = cv_rho;
    params.set<std::vector<VariableName>>("e") = cv_e;
    params.set<std::vector<VariableName>>("vel") = cv_vel;
    params.set<MaterialPropertyName>("visc_mu") = MU;
    params.set<MaterialPropertyName>("visc_kappa") = KAPPA;
    _m_sim.addKernel(class_name, Component::genName(comp_name, "evm", var_name[i]), params);
  }
}

void
EntropyViscosity::setup2Phase(FlowModelTwoPhase & fm, InputParameters & pars) const
{
  const auto & blocks = pars.get<std::vector<SubdomainName>>("block");
  Component * comp = pars.getCheckedPointerParam<Component *>("component");
  std::string comp_name = comp->name();

  // coupling vectors
  std::vector<VariableName> cv_beta(1, FlowModelTwoPhase::BETA);
  std::vector<VariableName> cv_arhoA_liquid(1, FlowModelTwoPhase::ALPHA_RHO_A_LIQUID);
  std::vector<VariableName> cv_arhouA_liquid(1, FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID);
  std::vector<VariableName> cv_arhoEA_liquid(1, FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID);
  std::vector<VariableName> cv_arhoA_vapor(1, FlowModelTwoPhase::ALPHA_RHO_A_VAPOR);
  std::vector<VariableName> cv_arhouA_vapor(1, FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR);
  std::vector<VariableName> cv_arhoEA_vapor(1, FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR);

  std::vector<VariableName> cv_density_liquid(1, FlowModelTwoPhase::DENSITY_LIQUID);
  std::vector<VariableName> cv_rhou_liquid(1, FlowModelTwoPhase::MOMENTUM_DENSITY_LIQUID);
  std::vector<VariableName> cv_rhoE_liquid(1, FlowModelTwoPhase::TOTAL_ENERGY_DENSITY_LIQUID);
  std::vector<VariableName> cv_e_liquid(1, FlowModelTwoPhase::SPECIFIC_INTERNAL_ENERGY_LIQUID);
  std::vector<VariableName> cv_pressure_liquid(1, FlowModelTwoPhase::PRESSURE_LIQUID);
  std::vector<VariableName> cv_vel_liquid(1, FlowModelTwoPhase::VELOCITY_LIQUID);
  std::vector<VariableName> cv_alpha_liquid(1, FlowModelTwoPhase::VOLUME_FRACTION_LIQUID);
  std::vector<VariableName> cv_jump_press_liquid(1, JUMP_PRESSURE_LIQUID);
  std::vector<VariableName> cv_jump_dens_liquid(1, JUMP_RHO_LIQUID);
  std::vector<VariableName> cv_jump_alpha(1, JUMP_ALPHA);

  std::vector<VariableName> cv_density_vapor(1, FlowModelTwoPhase::DENSITY_VAPOR);
  std::vector<VariableName> cv_rhou_vapor(1, FlowModelTwoPhase::MOMENTUM_DENSITY_VAPOR);
  std::vector<VariableName> cv_rhoE_vapor(1, FlowModelTwoPhase::TOTAL_ENERGY_DENSITY_VAPOR);
  std::vector<VariableName> cv_e_vapor(1, FlowModelTwoPhase::SPECIFIC_INTERNAL_ENERGY_VAPOR);
  std::vector<VariableName> cv_pressure_vapor(1, FlowModelTwoPhase::PRESSURE_VAPOR);
  std::vector<VariableName> cv_vel_vapor(1, FlowModelTwoPhase::VELOCITY_VAPOR);
  std::vector<VariableName> cv_alpha_vapor(1, FlowModelTwoPhase::VOLUME_FRACTION_VAPOR);
  std::vector<VariableName> cv_jump_press_vapor(1, JUMP_PRESSURE_VAPOR);
  std::vector<VariableName> cv_jump_dens_vapor(1, JUMP_RHO_VAPOR);

  std::vector<VariableName> cv_area(1, FlowModel::AREA);

  bool phase_interaction = fm.getPhaseInteraction();

  // output
  std::vector<OutputName> outputs = _m_sim.getOutputsVector("none");

  ExecFlagEnum execute_options(MooseUtils::getDefaultExecFlagEnum());
  execute_options = {EXEC_TIMESTEP_BEGIN, EXEC_INITIAL};

  if (_use_jump_vf)
  {
    // Jump of void fraction
    std::string class_name = "JumpGradientInterface";
    InputParameters params = _m_factory.getValidParams(class_name);
    params.set<std::vector<VariableName>>("variable") = cv_alpha_liquid;
    params.set<ExecFlagEnum>("execute_on") = execute_options;
    params.set<std::string>("jump") = JUMP_ALPHA;
    params.set<std::vector<SubdomainName>>("block") = blocks;
    _m_sim.addUserObject(class_name, Component::genName(comp_name, "jump_alpha"), params);
  }

  if (_use_jump_liquid)
  {
    // Jump of liquid pressure gradient.
    {
      std::string class_name = "JumpGradientInterface";
      InputParameters params = _m_factory.getValidParams(class_name);
      params.set<std::vector<VariableName>>("variable") = cv_pressure_liquid;
      params.set<ExecFlagEnum>("execute_on") = execute_options;
      params.set<std::string>("jump") = JUMP_PRESSURE_LIQUID;
      params.set<std::vector<SubdomainName>>("block") = blocks;
      _m_sim.addUserObject(
          class_name, Component::genName(comp_name, "jump_pressure_liquid"), params);
    }
    // Jump of liquid density gradient.
    {
      std::string class_name = "JumpGradientInterface";
      InputParameters params = _m_factory.getValidParams(class_name);
      params.set<std::vector<VariableName>>("variable") = cv_density_liquid;
      params.set<ExecFlagEnum>("execute_on") = execute_options;
      params.set<std::string>("jump") = JUMP_RHO_LIQUID;
      params.set<std::vector<SubdomainName>>("block") = blocks;
      _m_sim.addUserObject(class_name, Component::genName(comp_name, "jump_rho_liquid"), params);
    }
  }

  if (_use_jump_vapor)
  {
    // Jump of vapor pressure gradient.
    {
      std::string class_name = "JumpGradientInterface";
      InputParameters params = _m_factory.getValidParams(class_name);
      params.set<std::vector<VariableName>>("variable") = cv_pressure_vapor;
      params.set<ExecFlagEnum>("execute_on") = execute_options;
      params.set<std::string>("jump") = JUMP_PRESSURE_VAPOR;
      params.set<std::vector<SubdomainName>>("block") = blocks;
      _m_sim.addUserObject(
          class_name, Component::genName(comp_name, "jump_pressure_vapor"), params);
    }
    // Jump of vapor density gradient.
    {
      std::string class_name = "JumpGradientInterface";
      InputParameters params = _m_factory.getValidParams(class_name);
      params.set<std::vector<VariableName>>("variable") = cv_density_vapor;
      params.set<ExecFlagEnum>("execute_on") = execute_options;
      params.set<std::string>("jump") = JUMP_RHO_VAPOR;
      params.set<std::vector<SubdomainName>>("block") = blocks;
      _m_sim.addUserObject(class_name, Component::genName(comp_name, "jump_rho_vapor"), params);
    }
  }

  //
  // Liquid phase
  //

  // Velocity pps: average value for the liquid velocity.
  std::string avg_vel_liquid_name = Component::genName(comp_name, "average_velocity_liquid");
  {
    std::string class_name = "ElementAverageValue";
    InputParameters params = _m_factory.getValidParams(class_name);
    params.set<std::vector<VariableName>>("variable") =
        std::vector<VariableName>(1, FlowModelTwoPhase::VELOCITY_LIQUID);
    params.set<std::vector<OutputName>>("outputs") = outputs;
    params.set<ExecFlagEnum>("execute_on") = execute_options;
    params.set<std::vector<SubdomainName>>("block") = blocks;
    _m_sim.addPostprocessor(class_name, avg_vel_liquid_name, params);
  }

  // Volume fraction pps for liquid phase: average value
  std::string avg_vf_liquid_name = Component::genName(comp_name, "average_alpha_liquid");
  {
    std::string class_name = "ElementAverageValue";
    InputParameters params = _m_factory.getValidParams(class_name);
    params.set<std::vector<VariableName>>("variable") =
        std::vector<VariableName>(1, FlowModelTwoPhase::VOLUME_FRACTION_LIQUID);
    params.set<std::vector<OutputName>>("outputs") = outputs;
    params.set<ExecFlagEnum>("execute_on") = execute_options;
    params.set<std::vector<SubdomainName>>("block") = blocks;
    _m_sim.addPostprocessor(class_name, avg_vf_liquid_name, params);
  }

  // Volume fraction pps for liquid phase: infinite norm
  std::string inf_norm_vf_liquid_name =
      Component::genName(comp_name, "infinite_norm_vol_fraction_liquid");
  {
    std::string class_name = "NodalMaxFromAverage";
    InputParameters params = _m_factory.getValidParams(class_name);
    params.set<std::vector<VariableName>>("variable") =
        std::vector<VariableName>(1, FlowModelTwoPhase::VOLUME_FRACTION_LIQUID);
    params.set<std::string>("average_name_pps") = avg_vf_liquid_name;
    params.set<std::vector<OutputName>>("outputs") = outputs;
    params.set<ExecFlagEnum>("execute_on") = execute_options;
    params.set<std::vector<SubdomainName>>("block") = blocks;
    _m_sim.addPostprocessor(class_name, inf_norm_vf_liquid_name, params);
  }

  // Normalization parameter for liquid phase
  std::string norm_param_liquid_name = Component::genName(comp_name, "norm_param_liquid");
  {
    std::string class_name = "NormalizationParameter";
    InputParameters params = _m_factory.getValidParams(class_name);
    params.set<ExecFlagEnum>("execute_on") = execute_options;
    params.set<Real>("M_threshold") = _M_thres_liquid;
    params.set<Real>("a") = _a_liquid;
    params.set<PostprocessorName>("avg_vel") = avg_vel_liquid_name;
    params.set<MooseEnum>("funct_type") = _mach_regime_function_liquid;
    _m_sim.addUserObject(class_name, norm_param_liquid_name, params);
  }

  // Viscosity coefficients for liquid phase
  {
    std::string class_name = "EntropyViscosityCoefficients7EqnMaterial";
    InputParameters params = _m_factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = blocks;

    params.set<Real>("Cmax") = _Cmax_liquid;
    params.set<Real>("Cjump") = _Cjump_liquid;
    params.set<Real>("Centropy") = _Centropy_liquid;
    params.set<Real>("Cmax_vf") = _Cmax_vf;
    params.set<Real>("Cjump_vf") = _Cjump_vf;
    params.set<Real>("Centropy_vf") = _Centropy_vf;

    params.set<bool>("use_first_order") = _use_first_order_liquid;
    params.set<bool>("use_first_order_vf") = _use_first_order_vf;
    params.set<bool>("use_low_mach_fix") = _use_low_mach_fix;
    params.set<bool>("use_parabolic_regularization") = _use_parabolic_regularization_liquid;
    params.set<bool>("use_jump") = _use_jump_liquid;
    params.set<bool>("use_jump_vf") = _use_jump_vf;
    params.set<bool>("is_liquid") = true;

    params.set<std::vector<VariableName>>("rho") = cv_density_liquid;
    params.set<std::vector<VariableName>>("rhou") = cv_rhou_liquid;
    params.set<std::vector<VariableName>>("rhoE") = cv_rhoE_liquid;
    params.set<std::vector<VariableName>>("p") = cv_pressure_liquid;
    params.set<std::vector<VariableName>>("alpha") = cv_alpha_liquid;

    if (_use_jump_liquid)
    {
      params.set<std::vector<VariableName>>("jump_pressure") = cv_jump_press_liquid;
      params.set<std::vector<VariableName>>("jump_density") = cv_jump_dens_liquid;
    }

    if (_use_jump_vf)
      params.set<std::vector<VariableName>>("jump_alpha") = cv_jump_alpha;

    params.set<MaterialPropertyName>("c") = FlowModelTwoPhase::SOUND_SPEED_LIQUID;
    if (phase_interaction)
      params.set<MaterialPropertyName>("vel_int") = FlowModelTwoPhase::VELOCITY_INTERFACIAL;
    else
      params.set<MaterialPropertyName>("vel_int") = FlowModelTwoPhase::VELOCITY_LIQUID;
    params.set<MaterialPropertyName>("evm_visc_max") = MAX_VISCOSITY_LIQUID;
    params.set<MaterialPropertyName>("evm_kappa") = KAPPA_LIQUID;
    params.set<MaterialPropertyName>("evm_mu") = MU_LIQUID;
    params.set<MaterialPropertyName>("evm_beta_max") = MAX_BETA_LIQUID;
    params.set<MaterialPropertyName>("evm_beta") = BETA_LIQUID;
    params.set<MaterialPropertyName>("direction") = FlowModel::DIRECTION;
    params.set<UserObjectName>("norm_param") = norm_param_liquid_name;
    params.set<PostprocessorName>("vf_PPS_name") = inf_norm_vf_liquid_name;
    _m_sim.addMaterial(class_name, Component::genName(comp_name, "ent_visc_coeff_liquid"), params);
  }

  //
  // Vapor phase
  //

  // Postprocessors for vapor phase: average value of the velocity.
  std::string avg_vel_vapor_name = Component::genName(comp_name, "average_velocity_vapor");
  {
    std::string class_name = "ElementAverageValue";
    InputParameters params = _m_factory.getValidParams(class_name);
    params.set<std::vector<VariableName>>("variable") =
        std::vector<VariableName>(1, FlowModelTwoPhase::VELOCITY_VAPOR);
    params.set<std::vector<OutputName>>("outputs") = outputs;
    params.set<ExecFlagEnum>("execute_on") = execute_options;
    params.set<std::vector<SubdomainName>>("block") = blocks;
    _m_sim.addPostprocessor(class_name, avg_vel_vapor_name, params);
  }

  // Normalization parameter for vapor phase
  std::string norm_param_vapor_name = Component::genName(comp_name, "norm_param_vapor");
  {
    std::string class_name = "NormalizationParameter";
    InputParameters params = _m_factory.getValidParams(class_name);
    params.set<ExecFlagEnum>("execute_on") = execute_options;
    params.set<Real>("M_threshold") = _M_thres_vapor;
    params.set<Real>("a") = _a_vapor;
    params.set<PostprocessorName>("avg_vel") = avg_vel_vapor_name;
    params.set<MooseEnum>("funct_type") = _mach_regime_function_vapor;
    _m_sim.addUserObject(class_name, norm_param_vapor_name, params);
  }

  // Viscosity coefficients for vapor phase
  {
    std::string class_name = "EntropyViscosityCoefficients7EqnMaterial";
    InputParameters params = _m_factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = blocks;

    params.set<Real>("Cmax") = _Cmax_vapor;
    params.set<Real>("Cjump") = _Cjump_vapor;
    params.set<Real>("Centropy") = _Centropy_vapor;
    params.set<Real>("Cmax_vf") = _Cmax_vf;
    params.set<Real>("Cjump_vf") = _Cjump_vf;
    params.set<Real>("Centropy_vf") = _Centropy_vf;

    params.set<bool>("use_first_order") = _use_first_order_vapor;
    params.set<bool>("use_first_order_vf") = _use_first_order_vf;
    params.set<bool>("use_low_mach_fix") = _use_low_mach_fix;
    params.set<bool>("use_parabolic_regularization") = _use_parabolic_regularization_vapor;
    params.set<bool>("use_jump") = _use_jump_vapor;
    params.set<bool>("use_jump_vf") = _use_jump_vf;
    params.set<bool>("is_liquid") = false;

    params.set<std::vector<VariableName>>("rho") = cv_density_vapor;
    params.set<std::vector<VariableName>>("rhou") = cv_rhou_vapor;
    params.set<std::vector<VariableName>>("rhoE") = cv_rhoE_vapor;
    params.set<std::vector<VariableName>>("p") = cv_pressure_vapor;
    params.set<std::vector<VariableName>>("alpha") = cv_alpha_vapor;

    if (_use_jump_vapor)
    {
      params.set<std::vector<VariableName>>("jump_pressure") = cv_jump_press_vapor;
      params.set<std::vector<VariableName>>("jump_density") = cv_jump_dens_vapor;
    }

    if (_use_jump_vf)
      params.set<std::vector<VariableName>>("jump_alpha") = cv_jump_alpha;

    params.set<MaterialPropertyName>("c") = FlowModelTwoPhase::SOUND_SPEED_VAPOR;
    if (phase_interaction)
      params.set<MaterialPropertyName>("vel_int") = FlowModelTwoPhase::VELOCITY_INTERFACIAL;
    else
      params.set<MaterialPropertyName>("vel_int") = FlowModelTwoPhase::VELOCITY_VAPOR;
    params.set<MaterialPropertyName>("evm_visc_max") = MAX_VISCOSITY_VAPOR;
    params.set<MaterialPropertyName>("evm_kappa") = KAPPA_VAPOR;
    params.set<MaterialPropertyName>("evm_mu") = MU_VAPOR;
    params.set<MaterialPropertyName>("evm_beta_max") = MAX_BETA_VAPOR;
    params.set<MaterialPropertyName>("evm_beta") = BETA_VAPOR;
    params.set<MaterialPropertyName>("direction") = FlowModel::DIRECTION;
    params.set<UserObjectName>("norm_param") = norm_param_vapor_name;
    params.set<PostprocessorName>("vf_PPS_name") = inf_norm_vf_liquid_name;
    _m_sim.addMaterial(class_name, Component::genName(comp_name, "ent_visc_coeff_vapor"), params);
  }

  //
  // Kernels
  //

  std::vector<std::string> eqn_name(6, "");
  eqn_name[0] = "CONTINUITY";
  eqn_name[1] = "MOMENTUM";
  eqn_name[2] = "ENERGY";
  eqn_name[3] = "CONTINUITY";
  eqn_name[4] = "MOMENTUM";
  eqn_name[5] = "ENERGY";
  if (phase_interaction)
    eqn_name.push_back("VOIDFRACTION");

  std::vector<NonlinearVariableName> var_name(6, "");
  var_name[0] = FlowModelTwoPhase::ALPHA_RHO_A_LIQUID;
  var_name[1] = FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID;
  var_name[2] = FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID;
  var_name[3] = FlowModelTwoPhase::ALPHA_RHO_A_VAPOR;
  var_name[4] = FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR;
  var_name[5] = FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR;
  if (phase_interaction)
    var_name.push_back(FlowModelTwoPhase::BETA);

  for (unsigned int i = 0; i < var_name.size(); i++)
  {
    std::string class_name = "OneD7EqnEntropyMinimumArtificialDissipation";
    InputParameters params = _m_factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = var_name[i];
    params.set<MooseEnum>("eqn_name") = FlowModel::getFlowEquationType(eqn_name[i]);
    if ((i == 3) || (i == 4) || (i == 5))
    {
      params.set<bool>("is_liquid") = false;
      params.set<MaterialPropertyName>("alpha") = FlowModelTwoPhase::VOLUME_FRACTION_VAPOR;
      params.set<std::vector<VariableName>>("rho") = cv_density_vapor;
      params.set<std::vector<VariableName>>("e") = cv_e_vapor;
      params.set<std::vector<VariableName>>("vel") = cv_vel_vapor;

      params.set<std::vector<VariableName>>("beta") = cv_beta;
      params.set<std::vector<VariableName>>("arhoA") = cv_arhoA_vapor;
      params.set<std::vector<VariableName>>("arhouA") = cv_arhouA_vapor;
      params.set<std::vector<VariableName>>("arhoEA") = cv_arhoEA_vapor;

      params.set<MaterialPropertyName>("visc_beta") = BETA_LIQUID;
      params.set<MaterialPropertyName>("visc_mu") = MU_VAPOR;
      params.set<MaterialPropertyName>("visc_kappa") = KAPPA_VAPOR;
    }
    else
    {
      params.set<bool>("is_liquid") = true;
      params.set<MaterialPropertyName>("alpha") = FlowModelTwoPhase::VOLUME_FRACTION_LIQUID;
      params.set<std::vector<VariableName>>("rho") = cv_density_liquid;
      params.set<std::vector<VariableName>>("e") = cv_e_liquid;
      params.set<std::vector<VariableName>>("vel") = cv_vel_liquid;

      params.set<std::vector<VariableName>>("beta") = cv_beta;
      params.set<std::vector<VariableName>>("arhoA") = cv_arhoA_liquid;
      params.set<std::vector<VariableName>>("arhouA") = cv_arhouA_liquid;
      params.set<std::vector<VariableName>>("arhoEA") = cv_arhoEA_liquid;

      params.set<MaterialPropertyName>("visc_beta") = BETA_LIQUID;
      params.set<MaterialPropertyName>("visc_mu") = MU_LIQUID;
      params.set<MaterialPropertyName>("visc_kappa") = KAPPA_LIQUID;
    }
    params.set<std::vector<SubdomainName>>("block") = blocks;
    params.set<std::vector<VariableName>>("A") = cv_area;
    _m_sim.addKernel(class_name, Component::genName(comp_name, "evm", var_name[i]), params);
  }
}
