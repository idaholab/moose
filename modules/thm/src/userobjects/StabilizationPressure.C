#include "StabilizationPressure.h"
#include "FlowModel.h"
#include "Simulation.h"
#include "Component.h"
#include "FluidProperties.h"
#include "TwoPhaseFluidProperties.h"
#include "InputParameterLogic.h"
#include "FlowModelSinglePhase.h"
#include "FlowModelTwoPhase.h"

const std::string StabilizationPressure::PRESSURE_BAR = "p_bar";
const std::string StabilizationPressure::PRESSURE_BAR_LIQUID = "p_bar_liquid";
const std::string StabilizationPressure::PRESSURE_BAR_VAPOR = "p_bar_vapor";
const std::string StabilizationPressure::LAPLACE_P = "laplace_p";
const std::string StabilizationPressure::LAPLACE_P_LIQUID = "laplace_p_liquid";
const std::string StabilizationPressure::LAPLACE_P_VAPOR = "laplace_p_vapor";

registerMooseObject("THMApp", StabilizationPressure);

template <>
InputParameters
validParams<StabilizationPressure>()
{
  InputParameters params = validParams<StabilizationSettings>();

  params.addParam<Real>("ce", 1.5, "Coefficient for single phase.");
  params.addParam<Real>("ce_liquid", 0.5, "Coefficient for liquid phase.");
  params.addParam<Real>("ce_vapor", 0.5, "Coefficient for vapor phase.");

  params.addParam<Real>("scaling_factor_laplacep", 1e-5, "Scaling factor for Laplace(p)");
  params.addParam<Real>(
      "scaling_factor_laplacep_liquid", 1e-5, "Scaling factor for Laplace(p) for liquid phase");
  params.addParam<Real>(
      "scaling_factor_laplacep_vapor", 1e-5, "Scaling factor for Laplace(p) for vapor phase");

  MooseEnum pressure_smoother_method("FIRST SECOND");
  params.addRequiredParam<MooseEnum>(
      "order", pressure_smoother_method, "The pressure smoother method");

  params.addParam<Real>("p_reference", "Reference pressure for normalization");

  params.addParam<bool>("use_low_mach_fix", true, "Use the low-Mach fix");

  return params;
}

StabilizationPressure::StabilizationPressure(const InputParameters & parameters)
  : StabilizationSettings(parameters),
    _ce(getParam<Real>("ce")),
    _ce_liquid(getParam<Real>("ce_liquid")),
    _ce_vapor(getParam<Real>("ce_vapor")),
    _scaling_factor_laplacep(getParam<Real>("scaling_factor_laplacep")),
    _scaling_factor_laplacep_liquid(getParam<Real>("scaling_factor_laplacep_liquid")),
    _scaling_factor_laplacep_vapor(getParam<Real>("scaling_factor_laplacep_vapor")),
    _method_moose_enum(getParam<MooseEnum>("order")),
    _method(stringToEnum(_method_moose_enum)),
    _use_reference_pressure(isParamValid("p_reference")),
    _p_reference(_use_reference_pressure ? getParam<Real>("p_reference") : 0.0),
    _use_low_mach_fix(getParam<bool>("use_low_mach_fix"))
{
}

void
StabilizationPressure::addVariables(FlowModel & fm, unsigned int subdomain_id) const
{
  if (dynamic_cast<FlowModelSinglePhase *>(&fm))
  {
    _m_sim.addVariable(false, PRESSURE_BAR, FEType(CONSTANT, MONOMIAL), subdomain_id);
    if (_method == SECOND_ORDER)
      _m_sim.addVariable(false, LAPLACE_P, FlowModel::feType(), subdomain_id);
  }
  else if (dynamic_cast<FlowModelTwoPhase *>(&fm))
  {
    _m_sim.addVariable(false, PRESSURE_BAR_LIQUID, FEType(CONSTANT, MONOMIAL), subdomain_id);
    _m_sim.addVariable(false, PRESSURE_BAR_VAPOR, FEType(CONSTANT, MONOMIAL), subdomain_id);
    if (_method == SECOND_ORDER)
    {
      _m_sim.addVariable(false,
                         LAPLACE_P_LIQUID,
                         FlowModel::feType(),
                         subdomain_id,
                         _scaling_factor_laplacep_liquid);
      _m_sim.addVariable(false,
                         LAPLACE_P_VAPOR,
                         FlowModel::feType(),
                         subdomain_id,
                         _scaling_factor_laplacep_vapor);
    }
  }
}

void
StabilizationPressure::initMooseObjects(FlowModel & fm)
{
  bool isTwoPhase = dynamic_cast<FlowModelTwoPhase *>(&fm) != NULL;
  getOneOrTwoPhaseParameters<Real>(isTwoPhase, "ce", {"ce_liquid", "ce_vapor"}, *this);
}

void
StabilizationPressure::addMooseObjects(FlowModel & fm, InputParameters & pars) const
{
  if (dynamic_cast<FlowModelSinglePhase *>(&fm) != NULL)
    setup1Phase(dynamic_cast<FlowModelSinglePhase &>(fm), pars);
  else if (dynamic_cast<FlowModelTwoPhase *>(&fm) != NULL)
    setup2Phase(dynamic_cast<FlowModelTwoPhase &>(fm), pars);
  else
    mooseError("Pressure based stabilization is not implemented for selected model.");
}

void
StabilizationPressure::setup1Phase(FlowModelSinglePhase & /*fm*/, InputParameters & pars) const
{
  UserObjectName fp_name = pars.get<UserObjectName>("fp");
  Component * comp = pars.getCheckedPointerParam<Component *>("component");
  std::string comp_name = comp->name();
  const auto & blocks = pars.get<std::vector<SubdomainName>>("block");

  std::vector<NonlinearVariableName> vars;
  vars.push_back(FlowModelSinglePhase::RHOA);
  vars.push_back(FlowModelSinglePhase::RHOUA);
  vars.push_back(FlowModelSinglePhase::RHOEA);

  std::vector<VariableName> U;
  U.push_back(FlowModelSinglePhase::DENSITY);
  U.push_back(FlowModelSinglePhase::MOMENTUM_DENSITY);
  U.push_back(FlowModelSinglePhase::TOTAL_ENERGY_DENSITY);

  std::vector<VariableName> cv_rhoA(1, FlowModelSinglePhase::RHOA);
  std::vector<VariableName> cv_rhouA(1, FlowModelSinglePhase::RHOUA);
  std::vector<VariableName> cv_rhoEA(1, FlowModelSinglePhase::RHOEA);
  std::vector<VariableName> cv_area(1, FlowModelSinglePhase::AREA);
  std::vector<VariableName> cv_v(1, FlowModelSinglePhase::SPECIFIC_VOLUME);
  std::vector<VariableName> cv_e(1, FlowModelSinglePhase::SPECIFIC_INTERNAL_ENERGY);
  std::vector<VariableName> cv_rho(1, FlowModelSinglePhase::DENSITY);
  std::vector<VariableName> cv_rhou(1, FlowModelSinglePhase::MOMENTUM_DENSITY);
  std::vector<VariableName> cv_rhoE(1, FlowModelSinglePhase::TOTAL_ENERGY_DENSITY);
  std::vector<VariableName> cv_vel(1, FlowModelSinglePhase::VELOCITY);
  std::vector<VariableName> cv_p(1, FlowModelSinglePhase::PRESSURE);
  std::vector<VariableName> cv_p_bar(1, PRESSURE_BAR);
  std::vector<VariableName> cv_laplace_p(1, LAPLACE_P);

  switch (_method)
  {
    case FIRST_ORDER:
    {
      std::string class_name = "GradientPressureSmootherCoefMaterial";
      InputParameters params = _m_factory.getValidParams(class_name);
      params.set<std::vector<SubdomainName>>("block") = blocks;
      params.set<Real>("Ce") = _ce;
      params.set<MaterialPropertyName>("c") = "c";
      params.set<std::vector<VariableName>>("vel") = cv_vel;
      params.set<std::vector<VariableName>>("p") = cv_p;
      params.set<std::vector<VariableName>>("p_bar") = cv_p_bar;
      if (_use_reference_pressure)
        params.set<Real>("p_reference") = _p_reference;
      params.set<bool>("use_low_mach_fix") = _use_low_mach_fix;
      params.set<MaterialPropertyName>("coef_name") = "pressure_smoother_coef";
      params.set<MaterialPropertyName>("direction") = FlowModel::DIRECTION;
      _m_sim.addMaterial(class_name, Component::genName(comp_name, "p1_material"), params);

      break;
    }
    case SECOND_ORDER:
    {
      {
        std::string class_name = "LaplacianPressureSmootherCoefMaterial";
        InputParameters params = _m_factory.getValidParams(class_name);
        params.set<std::vector<SubdomainName>>("block") = blocks;
        params.set<Real>("Ce") = _ce;
        params.set<MaterialPropertyName>("c") = "c";
        params.set<std::vector<VariableName>>("vel") = cv_vel;
        params.set<std::vector<VariableName>>("p_bar") = cv_p_bar;
        params.set<std::vector<VariableName>>("laplace_p") = cv_laplace_p;
        if (_use_reference_pressure)
          params.set<Real>("p_reference") = _p_reference;
        params.set<bool>("use_low_mach_fix") = _use_low_mach_fix;
        params.set<MaterialPropertyName>("coef_name") = "pressure_smoother_coef";
        _m_sim.addMaterial(class_name, Component::genName(comp_name, "p2_material"), params);
      }

      {
        std::string class_name = "LaplaceProjectionTransfer";
        InputParameters params = _m_factory.getValidParams(class_name);
        params.set<AuxVariableName>("variable") = LAPLACE_P;
        params.set<VariableName>("source_variable") = FlowModelSinglePhase::PRESSURE;
        _m_sim.addTransfer(class_name, Component::genName(comp_name, "transfer"), params);
      }

      break;
    }

    default:
      mooseError("Invalid pressure smoother method.");
  }

  // Add viscous fluxes to the equations
  std::vector<NonlinearVariableName> var_name = {
      FlowModelSinglePhase::RHOA, FlowModelSinglePhase::RHOUA, FlowModelSinglePhase::RHOEA};
  std::vector<std::string> eqn_name = {"CONTINUITY", "MOMENTUM", "ENERGY"};

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
    params.set<MaterialPropertyName>("visc_mu") = "pressure_smoother_coef";
    params.set<MaterialPropertyName>("visc_kappa") = "pressure_smoother_coef";
    _m_sim.addKernel(class_name, Component::genName(comp_name, "evm", var_name[i]), params);
  }

  // aux kernels
  {
    std::string class_name = "PressureAux";
    InputParameters params = _m_factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = PRESSURE_BAR;
    params.set<std::vector<SubdomainName>>("block") = blocks;
    params.set<std::vector<VariableName>>("e") = cv_e;
    params.set<std::vector<VariableName>>("v") = cv_v;
    params.set<UserObjectName>("fp") = fp_name;
    _m_sim.addAuxKernel(class_name, Component::genName(comp_name, "p_bar_auxkernel"), params);
  }
}

void
StabilizationPressure::setup2Phase(FlowModelTwoPhase & fm, InputParameters & pars) const
{
  const auto & blocks = pars.get<std::vector<SubdomainName>>("block");
  Component * comp = pars.getCheckedPointerParam<Component *>("component");
  std::string comp_name = comp->name();
  bool phase_interaction = fm.getPhaseInteraction();
  UserObjectName fp = pars.get<UserObjectName>("fp");
  const TwoPhaseFluidProperties & tpfp = _m_sim.getUserObject<TwoPhaseFluidProperties>(fp);
  UserObjectName fp_liquid = tpfp.getLiquidName();
  UserObjectName fp_vapor = tpfp.getVaporName();

  std::vector<NonlinearVariableName> vars = {FlowModelTwoPhase::ALPHA_RHO_A_LIQUID,
                                             FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID,
                                             FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID,
                                             FlowModelTwoPhase::ALPHA_RHO_A_VAPOR,
                                             FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR,
                                             FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR};

  std::vector<VariableName> U = {FlowModelTwoPhase::DENSITY_LIQUID,
                                 FlowModelTwoPhase::MOMENTUM_DENSITY_LIQUID,
                                 FlowModelTwoPhase::TOTAL_ENERGY_DENSITY_LIQUID,
                                 FlowModelTwoPhase::DENSITY_VAPOR,
                                 FlowModelTwoPhase::MOMENTUM_DENSITY_VAPOR,
                                 FlowModelTwoPhase::TOTAL_ENERGY_DENSITY_VAPOR};

  std::vector<VariableName> cv_area(1, FlowModel::AREA);
  std::vector<VariableName> cv_beta(1, FlowModelTwoPhase::BETA);

  std::vector<VariableName> cv_alpha_liquid(1, FlowModelTwoPhase::VOLUME_FRACTION_LIQUID);
  std::vector<VariableName> cv_alpharhoA_liquid(1, FlowModelTwoPhase::ALPHA_RHO_A_LIQUID);
  std::vector<VariableName> cv_alpharhouA_liquid(1, FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID);
  std::vector<VariableName> cv_alpharhoEA_liquid(1, FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID);
  std::vector<VariableName> cv_density_liquid(1, FlowModelTwoPhase::DENSITY_LIQUID);
  std::vector<VariableName> cv_rhou_liquid(1, FlowModelTwoPhase::MOMENTUM_DENSITY_LIQUID);
  std::vector<VariableName> cv_rhoE_liquid(1, FlowModelTwoPhase::TOTAL_ENERGY_DENSITY_LIQUID);
  std::vector<VariableName> cv_v_liquid(1, FlowModelTwoPhase::SPECIFIC_VOLUME_LIQUID);
  std::vector<VariableName> cv_e_liquid(1, FlowModelTwoPhase::SPECIFIC_INTERNAL_ENERGY_LIQUID);
  std::vector<VariableName> cv_vel_liquid(1, FlowModelTwoPhase::VELOCITY_LIQUID);
  std::vector<VariableName> cv_p_liquid(1, FlowModelTwoPhase::PRESSURE_LIQUID);
  std::vector<VariableName> cv_p_bar_liquid(1, PRESSURE_BAR_LIQUID);
  std::vector<VariableName> cv_laplace_p_liquid(1, LAPLACE_P_LIQUID);

  std::vector<VariableName> cv_alpha_vapor(1, FlowModelTwoPhase::VOLUME_FRACTION_VAPOR);
  std::vector<VariableName> cv_alpharhoA_vapor(1, FlowModelTwoPhase::ALPHA_RHO_A_VAPOR);
  std::vector<VariableName> cv_alpharhouA_vapor(1, FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR);
  std::vector<VariableName> cv_alpharhoEA_vapor(1, FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR);
  std::vector<VariableName> cv_density_vapor(1, FlowModelTwoPhase::DENSITY_VAPOR);
  std::vector<VariableName> cv_rhou_vapor(1, FlowModelTwoPhase::MOMENTUM_DENSITY_VAPOR);
  std::vector<VariableName> cv_rhoE_vapor(1, FlowModelTwoPhase::TOTAL_ENERGY_DENSITY_VAPOR);
  std::vector<VariableName> cv_v_vapor(1, FlowModelTwoPhase::SPECIFIC_VOLUME_VAPOR);
  std::vector<VariableName> cv_e_vapor(1, FlowModelTwoPhase::SPECIFIC_INTERNAL_ENERGY_VAPOR);
  std::vector<VariableName> cv_vel_vapor(1, FlowModelTwoPhase::VELOCITY_VAPOR);
  std::vector<VariableName> cv_p_vapor(1, FlowModelTwoPhase::PRESSURE_VAPOR);
  std::vector<VariableName> cv_p_bar_vapor(1, PRESSURE_BAR_VAPOR);
  std::vector<VariableName> cv_laplace_p_vapor(1, LAPLACE_P_VAPOR);

  switch (_method)
  {
    case FIRST_ORDER:
    {
      {
        std::string class_name = "GradientPressureSmootherCoefMaterial";
        InputParameters params = _m_factory.getValidParams(class_name);
        params.set<std::vector<SubdomainName>>("block") = blocks;
        params.set<Real>("Ce") = _ce_liquid;
        params.set<MaterialPropertyName>("c") = "c_liquid";
        params.set<std::vector<VariableName>>("vel") = cv_vel_liquid;
        params.set<std::vector<VariableName>>("p") = cv_p_liquid;
        params.set<std::vector<VariableName>>("p_bar") = cv_p_bar_liquid;
        if (_use_reference_pressure)
          params.set<Real>("p_reference") = _p_reference;
        params.set<bool>("use_low_mach_fix") = _use_low_mach_fix;
        params.set<MaterialPropertyName>("coef_name") = "pressure_smoother_coef_liquid";
        params.set<MaterialPropertyName>("direction") = FlowModel::DIRECTION;
        _m_sim.addMaterial(class_name, Component::genName(comp_name, "p1_l_material"), params);
      }
      {
        std::string class_name = "GradientPressureSmootherCoefMaterial";
        InputParameters params = _m_factory.getValidParams(class_name);
        params.set<std::vector<SubdomainName>>("block") = blocks;
        params.set<Real>("Ce") = _ce_vapor;
        params.set<MaterialPropertyName>("c") = "c_vapor";
        params.set<std::vector<VariableName>>("vel") = cv_vel_vapor;
        params.set<std::vector<VariableName>>("p") = cv_p_vapor;
        params.set<std::vector<VariableName>>("p_bar") = cv_p_bar_vapor;
        if (_use_reference_pressure)
          params.set<Real>("p_reference") = _p_reference;
        params.set<bool>("use_low_mach_fix") = _use_low_mach_fix;
        params.set<MaterialPropertyName>("coef_name") = "pressure_smoother_coef_vapor";
        params.set<MaterialPropertyName>("direction") = FlowModel::DIRECTION;
        _m_sim.addMaterial(class_name, Component::genName(comp_name, "p1_v_material"), params);
      }
      {
        std::string class_name = "GradientPressureSmootherVolumeFractionCoefMaterial";
        InputParameters params = _m_factory.getValidParams(class_name);
        params.set<std::vector<SubdomainName>>("block") = blocks;
        params.set<Real>("Ce") = _ce_liquid;
        params.set<std::vector<VariableName>>("p") = cv_p_liquid;
        params.set<std::vector<VariableName>>("p_bar") = cv_p_bar_liquid;
        params.set<std::vector<VariableName>>("vel") = cv_vel_liquid;
        params.set<MaterialPropertyName>("c") = "c_liquid";
        params.set<MaterialPropertyName>("vel_int") = FlowModelTwoPhase::VELOCITY_INTERFACIAL;
        if (_use_reference_pressure)
          params.set<Real>("p_reference") = _p_reference;
        params.set<bool>("use_low_mach_fix") = _use_low_mach_fix;
        params.set<MaterialPropertyName>("coef_name") = "pressure_smoother_coef_vf_liquid";
        params.set<MaterialPropertyName>("direction") = FlowModel::DIRECTION;
        _m_sim.addMaterial(class_name, Component::genName(comp_name, "p1_coef_vf_liquid"), params);
      }
      {
        std::string class_name = "GradientPressureSmootherVolumeFractionCoefMaterial";
        InputParameters params = _m_factory.getValidParams(class_name);
        params.set<std::vector<SubdomainName>>("block") = blocks;
        params.set<Real>("Ce") = _ce_vapor;
        params.set<std::vector<VariableName>>("p") = cv_p_vapor;
        params.set<std::vector<VariableName>>("p_bar") = cv_p_bar_vapor;
        params.set<std::vector<VariableName>>("vel") = cv_vel_vapor;
        params.set<MaterialPropertyName>("c") = "c_vapor";
        params.set<MaterialPropertyName>("vel_int") = FlowModelTwoPhase::VELOCITY_INTERFACIAL;
        if (_use_reference_pressure)
          params.set<Real>("p_reference") = _p_reference;
        params.set<bool>("use_low_mach_fix") = _use_low_mach_fix;
        params.set<MaterialPropertyName>("coef_name") = "pressure_smoother_coef_vf_vapor";
        params.set<MaterialPropertyName>("direction") = FlowModel::DIRECTION;
        _m_sim.addMaterial(class_name, Component::genName(comp_name, "p1_coef_vf_vapor"), params);
      }

      break;
    }
    case SECOND_ORDER:
    {
      {
        std::string class_name = "LaplacianPressureSmootherCoefMaterial";
        InputParameters params = _m_factory.getValidParams(class_name);
        params.set<std::vector<SubdomainName>>("block") = blocks;
        params.set<Real>("Ce") = _ce_liquid;
        params.set<MaterialPropertyName>("c") = "c_liquid";
        params.set<std::vector<VariableName>>("vel") = cv_vel_liquid;
        params.set<std::vector<VariableName>>("p_bar") = cv_p_bar_liquid;
        params.set<std::vector<VariableName>>("laplace_p") = cv_laplace_p_liquid;
        if (_use_reference_pressure)
          params.set<Real>("p_reference") = _p_reference;
        params.set<bool>("use_low_mach_fix") = _use_low_mach_fix;
        params.set<MaterialPropertyName>("coef_name") = "pressure_smoother_coef_liquid";
        _m_sim.addMaterial(class_name, Component::genName(comp_name, "p2_l_material"), params);
      }
      {
        std::string class_name = "LaplacianPressureSmootherCoefMaterial";
        InputParameters params = _m_factory.getValidParams(class_name);
        params.set<std::vector<SubdomainName>>("block") = blocks;
        params.set<Real>("Ce") = _ce_vapor;
        params.set<MaterialPropertyName>("c") = "c_vapor";
        params.set<std::vector<VariableName>>("vel") = cv_vel_vapor;
        params.set<std::vector<VariableName>>("p_bar") = cv_p_bar_vapor;
        params.set<std::vector<VariableName>>("laplace_p") = cv_laplace_p_vapor;
        if (_use_reference_pressure)
          params.set<Real>("p_reference") = _p_reference;
        params.set<bool>("use_low_mach_fix") = _use_low_mach_fix;
        params.set<MaterialPropertyName>("coef_name") = "pressure_smoother_coef_vapor";
        _m_sim.addMaterial(class_name, Component::genName(comp_name, "p2_v_material"), params);
      }
      {
        std::string class_name = "LaplacianPressureSmootherVolumeFractionCoefMaterial";
        InputParameters params = _m_factory.getValidParams(class_name);
        params.set<std::vector<SubdomainName>>("block") = blocks;
        params.set<Real>("Ce") = _ce_liquid;
        params.set<std::vector<VariableName>>("p_bar") = cv_p_bar_liquid;
        params.set<std::vector<VariableName>>("laplace_p") = cv_laplace_p_liquid;
        params.set<std::vector<VariableName>>("vel") = cv_vel_liquid;
        params.set<MaterialPropertyName>("c") = "c_liquid";
        params.set<MaterialPropertyName>("vel_int") = FlowModelTwoPhase::VELOCITY_INTERFACIAL;
        if (_use_reference_pressure)
          params.set<Real>("p_reference") = _p_reference;
        params.set<bool>("use_low_mach_fix") = _use_low_mach_fix;
        params.set<MaterialPropertyName>("coef_name") = "pressure_smoother_coef_vf_liquid";
        _m_sim.addMaterial(class_name, Component::genName(comp_name, "p2_coef_vf_liquid"), params);
      }
      {
        std::string class_name = "LaplacianPressureSmootherVolumeFractionCoefMaterial";
        InputParameters params = _m_factory.getValidParams(class_name);
        params.set<std::vector<SubdomainName>>("block") = blocks;
        params.set<Real>("Ce") = _ce_liquid;
        params.set<std::vector<VariableName>>("p_bar") = cv_p_bar_vapor;
        params.set<std::vector<VariableName>>("laplace_p") = cv_laplace_p_vapor;
        params.set<std::vector<VariableName>>("vel") = cv_vel_vapor;
        params.set<MaterialPropertyName>("c") = "c_vapor";
        params.set<MaterialPropertyName>("vel_int") = FlowModelTwoPhase::VELOCITY_INTERFACIAL;
        if (_use_reference_pressure)
          params.set<Real>("p_reference") = _p_reference;
        params.set<bool>("use_low_mach_fix") = _use_low_mach_fix;
        params.set<MaterialPropertyName>("coef_name") = "pressure_smoother_coef_vf_vapor";
        _m_sim.addMaterial(class_name, Component::genName(comp_name, "p2_coef_vf_vapor"), params);
      }

      // create pressure Laplacian projection kernels
      {
        std::string class_name = "LaplaceProjectionTransfer";
        InputParameters params = _m_factory.getValidParams(class_name);
        params.set<AuxVariableName>("variable") = LAPLACE_P_LIQUID;
        params.set<VariableName>("source_variable") = FlowModelTwoPhase::PRESSURE_LIQUID;
        _m_sim.addTransfer(class_name, Component::genName(comp_name, "transfer_liquid"), params);
      }
      {
        std::string class_name = "LaplaceProjectionTransfer";
        InputParameters params = _m_factory.getValidParams(class_name);
        params.set<AuxVariableName>("variable") = LAPLACE_P_VAPOR;
        params.set<VariableName>("source_variable") = FlowModelTwoPhase::PRESSURE_VAPOR;
        _m_sim.addTransfer(class_name, Component::genName(comp_name, "transfer_vapor"), params);
      }

      break;
    }

    default:
      mooseError("Invalid pressure smoother method.");
  }

  std::vector<std::string> eqn_name = {
      "CONTINUITY", "MOMENTUM", "ENERGY", "CONTINUITY", "MOMENTUM", "ENERGY"};
  if (phase_interaction)
    eqn_name.push_back("VOIDFRACTION");

  std::vector<NonlinearVariableName> var_name = {FlowModelTwoPhase::ALPHA_RHO_A_LIQUID,
                                                 FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID,
                                                 FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID,
                                                 FlowModelTwoPhase::ALPHA_RHO_A_VAPOR,
                                                 FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR,
                                                 FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR};
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
      params.set<std::vector<VariableName>>("arhoA") = cv_alpharhoA_vapor;
      params.set<std::vector<VariableName>>("arhouA") = cv_alpharhouA_vapor;
      params.set<std::vector<VariableName>>("arhoEA") = cv_alpharhoEA_vapor;

      params.set<MaterialPropertyName>("visc_beta") = "pressure_smoother_coef_vf_vapor";
      params.set<MaterialPropertyName>("visc_mu") = "pressure_smoother_coef_vapor";
      params.set<MaterialPropertyName>("visc_kappa") = "pressure_smoother_coef_vapor";
    }
    else
    {
      params.set<bool>("is_liquid") = true;
      params.set<MaterialPropertyName>("alpha") = FlowModelTwoPhase::VOLUME_FRACTION_LIQUID;
      params.set<std::vector<VariableName>>("rho") = cv_density_liquid;
      params.set<std::vector<VariableName>>("e") = cv_e_liquid;
      params.set<std::vector<VariableName>>("vel") = cv_vel_liquid;

      params.set<std::vector<VariableName>>("beta") = cv_beta;
      params.set<std::vector<VariableName>>("arhoA") = cv_alpharhoA_liquid;
      params.set<std::vector<VariableName>>("arhouA") = cv_alpharhouA_liquid;
      params.set<std::vector<VariableName>>("arhoEA") = cv_alpharhoEA_liquid;

      params.set<MaterialPropertyName>("visc_beta") = "pressure_smoother_coef_vf_liquid";
      params.set<MaterialPropertyName>("visc_mu") = "pressure_smoother_coef_liquid";
      params.set<MaterialPropertyName>("visc_kappa") = "pressure_smoother_coef_liquid";
    }
    params.set<std::vector<SubdomainName>>("block") = blocks;
    params.set<std::vector<VariableName>>("A") = cv_area;
    _m_sim.addKernel(class_name, Component::genName(comp_name, "evm", var_name[i]), params);
  }

  // aux kernels

  {
    std::string class_name = "PressureAux";
    InputParameters params = _m_factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = PRESSURE_BAR_LIQUID;
    params.set<std::vector<SubdomainName>>("block") = blocks;
    params.set<std::vector<VariableName>>("e") = cv_e_liquid;
    params.set<std::vector<VariableName>>("v") = cv_v_liquid;
    params.set<UserObjectName>("fp") = fp_liquid;
    _m_sim.addAuxKernel(class_name, Component::genName(comp_name, "p_bar_l_auxkernel"), params);
  }
  {
    std::string class_name = "PressureAux";
    InputParameters params = _m_factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = PRESSURE_BAR_VAPOR;
    params.set<std::vector<SubdomainName>>("block") = blocks;
    params.set<std::vector<VariableName>>("e") = cv_e_vapor;
    params.set<std::vector<VariableName>>("v") = cv_v_vapor;
    params.set<UserObjectName>("fp") = fp_vapor;
    _m_sim.addAuxKernel(class_name, Component::genName(comp_name, "p_bar_v_auxkernel"), params);
  }
}

void
StabilizationPressure::initMethodTypeMap()
{
  if (_method_type_to_enum.empty())
  {
    _method_type_to_enum["FIRST"] = FIRST_ORDER;
    _method_type_to_enum["SECOND"] = SECOND_ORDER;
  }
}

StabilizationPressure::EPressureSmootherMethodType
StabilizationPressure::stringToEnum(const std::string & s)
{
  initMethodTypeMap();

  std::string upper(s);
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  if (!_method_type_to_enum.count(upper))
    mooseError(name(), ": The supplied value for parameter \"order\" is invalid: \"", upper, "\"");

  return _method_type_to_enum[upper];
}
