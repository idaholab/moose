#include "InletStagnationPressureTemperature.h"
#include "FlowModelSinglePhase.h"
#include "FlowModelTwoPhase.h"
#include "FlowModelTwoPhaseNCG.h"
#include "TwoPhaseFluidProperties.h"
#include "InputParameterLogic.h"

registerMooseObject("THMApp", InletStagnationPressureTemperature);

template <>
InputParameters
validParams<InletStagnationPressureTemperature>()
{
  InputParameters params = validParams<FlowBoundary>();
  // 1-phase
  params.addParam<Real>("p0", "Prescribed stagnation pressure");
  params.addParam<Real>("T0", "Prescribed stagnation temperature");
  // 2-phase
  params.addParam<Real>("p0_liquid", "Prescribed stagnation pressure for liquid phase");
  params.addParam<Real>("T0_liquid", "Prescribed stagnation temperature for liquid phase");
  params.addParam<Real>("p0_vapor", "Prescribed stagnation pressure for vapor phase");
  params.addParam<Real>("T0_vapor", "Prescribed stagnation temperature for vapor phase");
  params.addParam<Real>("alpha_vapor", "Prescribed vapor volume fraction");
  // ncg
  params.addParam<std::vector<Real>>("x_ncgs",
                                     "Prescribed mass fractions of non-condensable gases");

  params.addParam<bool>("reversible", false, "true for reversible, false (default) for pure inlet");

  return params;
}

InletStagnationPressureTemperature::InletStagnationPressureTemperature(
    const InputParameters & params)
  : FlowBoundary(params), _reversible(getParam<bool>("reversible"))
{
}

void
InletStagnationPressureTemperature::check() const
{
  FlowBoundary::check();

  // Input type checking
  if (_flow_model_id == THM::FM_SINGLE_PHASE)
  {
    for (auto p : {"p0", "T0"})
      check1PhaseRequiredParameter(p);
  }
  else if (_flow_model_id == THM::FM_TWO_PHASE || _flow_model_id == THM::FM_TWO_PHASE_NCG)
  {
    const FlowModelTwoPhase & fm = dynamic_cast<const FlowModelTwoPhase &>(*_flow_model);
    bool phase_interaction = fm.getPhaseInteraction();
    if (phase_interaction)
      check7EqnRequiredParameter("alpha_vapor");

    for (auto p : {"p0_liquid", "T0_liquid", "p0_vapor", "T0_vapor"})
      check2PhaseRequiredParameter(p);

    if (_flow_model_id == THM::FM_TWO_PHASE_NCG)
    {
      check7EqnNCGRequiredParameter("x_ncgs");

      if (isParamValid("x_ncgs"))
      {
        const FlowModelTwoPhaseNCG & fm = dynamic_cast<const FlowModelTwoPhaseNCG &>(*_flow_model);
        checkSizeEqualsValue<Real>("x_ncgs", fm.getNCGSolutionVars().size());
      }
    }
  }
  else
    logModelNotImplementedError(_flow_model_id);

  if ((FlowModel::getSpatialDiscretizationType() == FlowModel::rDG) &&
      (_flow_model_id == THM::FM_TWO_PHASE_NCG))
    logSpatialDiscretizationNotImplementedError(FlowModel::getSpatialDiscretizationType());
}

void
InletStagnationPressureTemperature::setup1PhaseCG()
{
  std::vector<VariableName> cv_rhoA(1, FlowModelSinglePhase::RHOA);
  std::vector<VariableName> cv_rhouA(1, FlowModelSinglePhase::RHOUA);
  std::vector<VariableName> cv_rhoEA(1, FlowModelSinglePhase::RHOEA);
  std::vector<VariableName> cv_vel(1, FlowModelSinglePhase::VELOCITY);
  std::vector<VariableName> cv_v(1, FlowModelSinglePhase::SPECIFIC_VOLUME);
  std::vector<VariableName> cv_e(1, FlowModelSinglePhase::SPECIFIC_INTERNAL_ENERGY);
  std::vector<VariableName> cv_rho(1, FlowModelSinglePhase::DENSITY);
  std::vector<VariableName> cv_area(1, FlowModel::AREA);

  Real p0 = getParam<Real>("p0");
  Real T0 = getParam<Real>("T0");

  {
    std::string class_name = "OneDMassStagnationPandTBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<UserObjectName>("fp") = _fp_name;
    params.set<bool>("reversible") = _reversible;
    params.set<Real>("normal") = _normal;
    params.set<Real>("T0") = T0;
    params.set<Real>("p0") = p0;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("vel") = cv_vel;
    params.set<std::vector<VariableName>>("arhoA") = cv_rhoA;
    params.set<std::vector<VariableName>>("arhouA") = cv_rhouA;
    params.set<std::vector<VariableName>>("arhoEA") = cv_rhoEA;
    std::string nm = genName(name(), "rhoA_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, "", nm, "p0");
    connectObject(params, "", nm, "T0");
  }
  {
    std::string class_name = "OneDMomentumStagnationPandTBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOUA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<UserObjectName>("fp") = _fp_name;
    params.set<bool>("reversible") = _reversible;
    params.set<Real>("T0") = T0;
    params.set<Real>("p0") = p0;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("vel") = cv_vel;
    params.set<MaterialPropertyName>("alpha") = FlowModel::UNITY;
    params.set<std::vector<VariableName>>("arhoA") = cv_rhoA;
    params.set<std::vector<VariableName>>("arhouA") = cv_rhouA;
    params.set<std::vector<VariableName>>("arhoEA") = cv_rhoEA;
    std::string nm = genName(name(), "rhouA_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, "", nm, "p0");
    connectObject(params, "", nm, "T0");
  }
  {
    std::string class_name = "OneDEnergyStagnationPandTBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<UserObjectName>("fp") = _fp_name;
    params.set<bool>("reversible") = _reversible;
    params.set<Real>("normal") = _normal;
    params.set<Real>("T0") = T0;
    params.set<Real>("p0") = p0;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("vel") = cv_vel;
    params.set<std::vector<VariableName>>("arhoA") = cv_rhoA;
    params.set<std::vector<VariableName>>("arhouA") = cv_rhouA;
    params.set<std::vector<VariableName>>("arhoEA") = cv_rhoEA;
    std::string nm = genName(name(), "rhoEA_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, "", nm, "p0");
    connectObject(params, "", nm, "T0");
  }

  if (_reversible)
  {
    {
      std::string class_name = "OneDMassFreeInletReverseBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOA;
      params.set<bool>("reversible") = _reversible;
      params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
      params.set<Real>("normal") = _normal;
      params.set<std::vector<VariableName>>("arhouA") = cv_rhouA;
      _sim.addBoundaryCondition(class_name, genName(name(), "rhoA", "rev"), params);
    }
    {
      std::string class_name = "OneDMomentumStaticPressureBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOUA;
      params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
      params.set<Real>("normal") = _normal;
      params.set<Real>("p_in") = p0;
      params.set<bool>("reversible") = _reversible;
      // coupling
      params.set<std::vector<VariableName>>("rhoA") = cv_rhoA;
      params.set<std::vector<VariableName>>("rhoEA") = cv_rhoEA;
      params.set<MaterialPropertyName>("alpha") = FlowModel::UNITY;
      params.set<std::vector<VariableName>>("vel") = cv_vel;
      params.set<std::vector<VariableName>>("A") = cv_area;
      std::string nm = genName(name(), "bc_rhou", "rev");
      _sim.addBoundaryCondition(class_name, nm, params);
      connectObject(params, "", nm, "p0", "p_in");
    }
    {
      std::string class_name = "OneDEnergyStaticPressureBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
      params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
      params.set<UserObjectName>("fp") = _fp_name;
      params.set<UserObjectName>("vfm") = FlowModelTwoPhase::VOLUME_FRACTION_MAPPER;
      params.set<bool>("is_liquid") = true;
      params.set<Real>("p_in") = p0;
      params.set<bool>("reversible") = _reversible;
      params.set<Real>("normal") = _normal;
      // coupling
      params.set<std::vector<VariableName>>("A") = cv_area;
      params.set<std::vector<VariableName>>("rho") = cv_rho;
      params.set<std::vector<VariableName>>("rhoA") = cv_rhoA;
      params.set<std::vector<VariableName>>("rhouA") = cv_rhouA;
      params.set<std::vector<VariableName>>("vel") = cv_vel;
      params.set<std::vector<VariableName>>("e") = cv_e;
      params.set<std::vector<VariableName>>("v") = cv_v;
      std::string nm = genName(name(), "bc_rhoE", "rev");
      _sim.addBoundaryCondition(class_name, nm, params);
      connectObject(params, "", nm, "p0", "p_in");
    }
  }
}

void
InletStagnationPressureTemperature::setup1PhaseRDG()
{
  ExecFlagEnum userobject_execute_on(MooseUtils::getDefaultExecFlagEnum());
  userobject_execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  // boundary flux user object
  const std::string boundary_flux_name = genName(name(), "boundary_flux");
  {
    const std::string class_name = "BoundaryFlux3EqnGhostStagnationPressureTemperature";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<Real>("p0") = getParam<Real>("p0");
    params.set<Real>("T0") = getParam<Real>("T0");
    params.set<UserObjectName>("fluid_properties") = _fp_name;
    params.set<UserObjectName>("numerical_flux") = _numerical_flux_name;
    params.set<ExecFlagEnum>("execute_on") = userobject_execute_on;
    _sim.addUserObject(class_name, boundary_flux_name, params);
  }

  // BCs
  addWeakBC3Eqn(boundary_flux_name);
}

void
InletStagnationPressureTemperature::setup2PhaseCG()
{
  // get names of fluid properties for each phase
  const TwoPhaseFluidProperties & two_phase_fp =
      _sim.getUserObject<TwoPhaseFluidProperties>(_fp_name);
  UserObjectName fp_liquid_name = two_phase_fp.getLiquidName();
  UserObjectName fp_vapor_name = two_phase_fp.getVaporName();

  std::vector<VariableName> cv_area(1, FlowModel::AREA);
  std::vector<VariableName> cv_beta(1, FlowModelTwoPhase::BETA);
  std::vector<VariableName> cv_arhoA_liquid(1, FlowModelTwoPhase::ALPHA_RHO_A_LIQUID);
  std::vector<VariableName> cv_arhouA_liquid(1, FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID);
  std::vector<VariableName> cv_arhoEA_liquid(1, FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID);
  std::vector<VariableName> cv_alpha_liquid(1, FlowModelTwoPhase::VOLUME_FRACTION_LIQUID);
  std::vector<VariableName> cv_vel_liquid(1, FlowModelTwoPhase::VELOCITY_LIQUID);
  std::vector<VariableName> cv_arhoA_vapor(1, FlowModelTwoPhase::ALPHA_RHO_A_VAPOR);
  std::vector<VariableName> cv_arhouA_vapor(1, FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR);
  std::vector<VariableName> cv_arhoEA_vapor(1, FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR);
  std::vector<VariableName> cv_alpha_vapor(1, FlowModelTwoPhase::VOLUME_FRACTION_VAPOR);
  std::vector<VariableName> cv_vel_vapor(1, FlowModelTwoPhase::VELOCITY_VAPOR);

  const Real p0_liquid = getParam<Real>("p0_liquid");
  const Real T0_liquid = getParam<Real>("T0_liquid");
  const Real p0_vapor = getParam<Real>("p0_vapor");
  const Real T0_vapor = getParam<Real>("T0_vapor");
  const Real alpha_vapor = getParam<Real>("alpha_vapor");

  const FlowModelTwoPhase & fm = dynamic_cast<const FlowModelTwoPhase &>(*_flow_model);
  bool phase_interaction = fm.getPhaseInteraction();

  // volume fraction BC: Dirichlet
  if (phase_interaction)
  {
    std::string class_name = "OneDVolumeFractionVaporBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::BETA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<UserObjectName>("vfm") = FlowModelTwoPhase::VOLUME_FRACTION_MAPPER;
    params.set<Real>("alpha") = alpha_vapor;
    std::string nm = genName(name(), "vf_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, "", nm, "alpha_vapor", "alpha");
  }

  // liquid BC
  {
    std::string class_name = "OneDMassStagnationPandTBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHO_A_LIQUID;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<UserObjectName>("fp") = fp_liquid_name;
    params.set<UserObjectName>("vfm") = FlowModelTwoPhase::VOLUME_FRACTION_MAPPER;
    params.set<Real>("T0") = T0_liquid;
    params.set<Real>("p0") = p0_liquid;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("vel") = cv_vel_liquid;
    params.set<std::vector<VariableName>>("alpha") = cv_alpha_liquid;
    params.set<std::vector<VariableName>>("beta") = cv_beta;
    params.set<std::vector<VariableName>>("arhoA") = cv_arhoA_liquid;
    params.set<std::vector<VariableName>>("arhouA") = cv_arhouA_liquid;
    params.set<std::vector<VariableName>>("arhoEA") = cv_arhoEA_liquid;
    params.set<bool>("is_liquid") = true;
    std::string nm = genName(name(), "arhoA_bc_liquid");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, "", nm, "p0_liquid", "p0");
    connectObject(params, "", nm, "T0_liquid", "T0");
  }
  {
    std::string class_name = "OneDMomentumStagnationPandTBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<UserObjectName>("fp") = fp_liquid_name;
    params.set<Real>("T0") = T0_liquid;
    params.set<Real>("p0") = p0_liquid;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("vel") = cv_vel_liquid;
    params.set<MaterialPropertyName>("alpha") = FlowModelTwoPhase::VOLUME_FRACTION_LIQUID;
    params.set<std::vector<VariableName>>("beta") = cv_beta;
    params.set<std::vector<VariableName>>("arhoA") = cv_arhoA_liquid;
    params.set<std::vector<VariableName>>("arhouA") = cv_arhouA_liquid;
    params.set<std::vector<VariableName>>("arhoEA") = cv_arhoEA_liquid;
    std::string nm = genName(name(), "arhouA_bc_liquid");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, "", nm, "p0_liquid", "p0");
    connectObject(params, "", nm, "T0_liquid", "T0");
  }
  {
    std::string class_name = "OneDEnergyStagnationPandTBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<UserObjectName>("fp") = fp_liquid_name;
    params.set<UserObjectName>("vfm") = FlowModelTwoPhase::VOLUME_FRACTION_MAPPER;
    params.set<Real>("T0") = T0_liquid;
    params.set<Real>("p0") = p0_liquid;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("vel") = cv_vel_liquid;
    params.set<std::vector<VariableName>>("alpha") = cv_alpha_liquid;
    params.set<std::vector<VariableName>>("beta") = cv_beta;
    params.set<std::vector<VariableName>>("arhoA") = cv_arhoA_liquid;
    params.set<std::vector<VariableName>>("arhouA") = cv_arhouA_liquid;
    params.set<std::vector<VariableName>>("arhoEA") = cv_arhoEA_liquid;
    params.set<bool>("is_liquid") = true;
    std::string nm = genName(name(), "arhoEA_bc_liquid");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, "", nm, "p0_liquid", "p0");
    connectObject(params, "", nm, "T0_liquid", "T0");
  }

  // vapor BC
  {
    std::string class_name = "OneDMassStagnationPandTBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHO_A_VAPOR;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<UserObjectName>("fp") = fp_vapor_name;
    params.set<UserObjectName>("vfm") = FlowModelTwoPhase::VOLUME_FRACTION_MAPPER;
    params.set<Real>("T0") = T0_vapor;
    params.set<Real>("p0") = p0_vapor;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("vel") = cv_vel_vapor;
    params.set<std::vector<VariableName>>("alpha") = cv_alpha_vapor;
    params.set<std::vector<VariableName>>("beta") = cv_beta;
    params.set<std::vector<VariableName>>("arhoA") = cv_arhoA_vapor;
    params.set<std::vector<VariableName>>("arhouA") = cv_arhouA_vapor;
    params.set<std::vector<VariableName>>("arhoEA") = cv_arhoEA_vapor;
    params.set<bool>("is_liquid") = false;
    std::string nm = genName(name(), "arhoA_bc_vapor");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, "", nm, "p0_vapor", "p0");
    connectObject(params, "", nm, "T0_vapor", "T0");
  }
  {
    std::string class_name = "OneDMomentumStagnationPandTBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<UserObjectName>("fp") = fp_vapor_name;
    params.set<Real>("T0") = T0_vapor;
    params.set<Real>("p0") = p0_vapor;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("vel") = cv_vel_vapor;
    params.set<MaterialPropertyName>("alpha") = FlowModelTwoPhase::VOLUME_FRACTION_VAPOR;
    params.set<std::vector<VariableName>>("beta") = cv_beta;
    params.set<std::vector<VariableName>>("arhoA") = cv_arhoA_vapor;
    params.set<std::vector<VariableName>>("arhouA") = cv_arhouA_vapor;
    params.set<std::vector<VariableName>>("arhoEA") = cv_arhoEA_vapor;
    std::string nm = genName(name(), "arhouA_bc_vapor");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, "", nm, "p0_vapor", "p0");
    connectObject(params, "", nm, "T0_vapor", "T0");
  }
  {
    std::string class_name = "OneDEnergyStagnationPandTBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<UserObjectName>("fp") = fp_vapor_name;
    params.set<UserObjectName>("vfm") = FlowModelTwoPhase::VOLUME_FRACTION_MAPPER;
    params.set<Real>("T0") = T0_vapor;
    params.set<Real>("p0") = p0_vapor;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("vel") = cv_vel_vapor;
    params.set<std::vector<VariableName>>("alpha") = cv_alpha_vapor;
    params.set<std::vector<VariableName>>("beta") = cv_beta;
    params.set<std::vector<VariableName>>("arhoA") = cv_arhoA_vapor;
    params.set<std::vector<VariableName>>("arhouA") = cv_arhouA_vapor;
    params.set<std::vector<VariableName>>("arhoEA") = cv_arhoEA_vapor;
    params.set<bool>("is_liquid") = false;
    std::string nm = genName(name(), "arhoEA_bc_vapor");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, "", nm, "p0_vapor", "p0");
    connectObject(params, "", nm, "T0_vapor", "T0");
  }
}

void
InletStagnationPressureTemperature::setup2PhaseRDG()
{
  ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
  execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  // boundary flux user object
  const std::string boundary_flux_name = genName(name(), "boundary_flux");
  {
    const std::string class_name = "BoundaryFlux7EqnGhostStagnationPressureTemperature";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<Real>("alpha_vapor") = getParam<Real>("alpha_vapor");
    params.set<Real>("p0_liquid") = getParam<Real>("p0_liquid");
    params.set<Real>("T0_liquid") = getParam<Real>("T0_liquid");
    params.set<Real>("p0_vapor") = getParam<Real>("p0_vapor");
    params.set<Real>("T0_vapor") = getParam<Real>("T0_vapor");
    params.set<UserObjectName>("fp_2phase") = _fp_name;
    params.set<UserObjectName>("numerical_flux") = _numerical_flux_name;
    params.set<UserObjectName>("vfm") = FlowModelTwoPhase::VOLUME_FRACTION_MAPPER;
    params.set<ExecFlagEnum>("execute_on") = execute_on;
    _sim.addUserObject(class_name, boundary_flux_name, params);
  }

  // BCs
  addWeakBC7Eqn(boundary_flux_name);
}

void
InletStagnationPressureTemperature::setup2PhaseNCGCG()
{
  setup2PhaseCG();

  const FlowModelTwoPhaseNCG & fm = dynamic_cast<const FlowModelTwoPhaseNCG &>(*_flow_model);
  const std::vector<VariableName> & vars = fm.getNCGSolutionVars();
  const std::vector<Real> & x_ncgs = getParam<std::vector<Real>>("x_ncgs");

  for (std::size_t i = 0; i < vars.size(); i++)
  {
    {
      std::string class_name = "OneDMassNCGSpecifiedMassFractionBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = vars[i];
      params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
      params.set<Real>("normal") = _normal;
      params.set<Real>("x_ncg") = x_ncgs[i];
      params.set<std::vector<VariableName>>("arhouA_vapor") = {
          FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR};
      _sim.addBoundaryCondition(class_name, genName(name(), "aXrhoA_vapor_bc"), params);
    }
  }
}

void
InletStagnationPressureTemperature::addMooseObjects()
{
  // apply logic for parameters with one- and two-phase variants
  const bool is_two_phase =
      _flow_model_id == THM::FM_TWO_PHASE || _flow_model_id == THM::FM_TWO_PHASE_NCG;
  getOneOrTwoPhaseParameters<Real>(is_two_phase, "p0", {"p0_liquid", "p0_vapor"}, *this);
  getOneOrTwoPhaseParameters<Real>(is_two_phase, "T0", {"T0_liquid", "T0_vapor"}, *this);

  if (_flow_model_id == THM::FM_SINGLE_PHASE)
  {
    if (FlowModel::getSpatialDiscretizationType() == FlowModel::CG)
      setup1PhaseCG();
    else if (FlowModel::getSpatialDiscretizationType() == FlowModel::rDG)
      setup1PhaseRDG();
  }
  else if (_flow_model_id == THM::FM_TWO_PHASE)
  {
    if (FlowModel::getSpatialDiscretizationType() == FlowModel::CG)
      setup2PhaseCG();
    else if (FlowModel::getSpatialDiscretizationType() == FlowModel::rDG)
      setup2PhaseRDG();
  }
  else if (_flow_model_id == THM::FM_TWO_PHASE_NCG)
    if (FlowModel::getSpatialDiscretizationType() == FlowModel::CG)
      setup2PhaseNCGCG();
}
