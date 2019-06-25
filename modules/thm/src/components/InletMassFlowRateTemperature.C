#include "InletMassFlowRateTemperature.h"
#include "FlowModelSinglePhase.h"
#include "FlowModelTwoPhase.h"
#include "FlowModelTwoPhaseNCG.h"
#include "TwoPhaseFluidProperties.h"
#include "InputParameterLogic.h"

registerMooseObject("THMApp", InletMassFlowRateTemperature);

template <>
InputParameters
validParams<InletMassFlowRateTemperature>()
{
  InputParameters params = validParams<FlowBoundary>();
  // 1-phase
  params.addParam<Real>("m_dot", "Prescribed mass flow rate");
  params.addParam<Real>("T", "prescribed temperature (used only in 3eqn model)");
  // 2-phase
  params.addParam<Real>("m_dot_liquid", "Prescribed mass flow rate of liquid");
  params.addParam<Real>("T_liquid", "Prescribed temperature of liquid");
  params.addParam<Real>("m_dot_vapor", "Prescribed mass flow rate of vapor");
  params.addParam<Real>("T_vapor", "Prescribed temperature of vapor");
  params.addParam<Real>("alpha_vapor", "Prescribed vapor volume fraction");
  // ncg
  params.addParam<std::vector<Real>>("x_ncgs",
                                     "Prescribed mass fractions of non-condensable gases");

  params.addParam<bool>("reversible", false, "true for reversible, false (default) for pure inlet");

  return params;
}

InletMassFlowRateTemperature::InletMassFlowRateTemperature(const InputParameters & params)
  : FlowBoundary(params), _reversible(getParam<bool>("reversible"))
{
}

void
InletMassFlowRateTemperature::check() const
{
  FlowBoundary::check();

  // Input type checking
  if (_flow_model_id == THM::FM_SINGLE_PHASE)
  {
    for (auto p : {"m_dot", "T"})
      check1PhaseRequiredParameter(p);
  }
  else if (_flow_model_id == THM::FM_TWO_PHASE || _flow_model_id == THM::FM_TWO_PHASE_NCG)
  {
    auto fm = dynamic_cast<const FlowModelTwoPhase *>(_flow_model.get());
    if (fm != nullptr)
    {
      bool phase_interaction = fm->getPhaseInteraction();
      if (phase_interaction)
        check7EqnRequiredParameter("alpha_vapor");
    }

    for (auto p : {"m_dot_liquid", "T_liquid", "m_dot_vapor", "T_vapor"})
      check2PhaseRequiredParameter(p);

    if (_flow_model_id == THM::FM_TWO_PHASE_NCG)
    {
      check7EqnNCGRequiredParameter("x_ncgs");

      if (isParamValid("x_ncgs"))
      {
        auto fm = dynamic_cast<const FlowModelTwoPhaseNCG *>(_flow_model.get());
        if (fm != nullptr)
          checkSizeEqualsValue<Real>("x_ncgs", fm->getNCGSolutionVars().size());
      }
    }
  }
  else
    logModelNotImplementedError(_flow_model_id);

  if ((_spatial_discretization == FlowModel::rDG) && (_flow_model_id == THM::FM_TWO_PHASE_NCG))
    logSpatialDiscretizationNotImplementedError(_spatial_discretization);
}

void
InletMassFlowRateTemperature::setup1PhaseCG()
{
  // coupling vectors
  std::vector<VariableName> cv_area(1, FlowModel::AREA);
  std::vector<VariableName> cv_rhoA(1, FlowModelSinglePhase::RHOA);
  std::vector<VariableName> cv_rhouA(1, FlowModelSinglePhase::RHOUA);
  std::vector<VariableName> cv_rhoEA(1, FlowModelSinglePhase::RHOEA);
  std::vector<VariableName> cv_pressure(1, FlowModelSinglePhase::PRESSURE);

  Real m_dot_in = getParam<Real>("m_dot");
  Real T_in = getParam<Real>("T");

  {
    std::string class_name = "OneDMassMassFlowRateTemperatureBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("m_dot") = m_dot_in;
    std::string nm = genName(name(), "rhoA_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "m_dot", "m_dot");
  }
  {
    std::string class_name = "OneDMomentumMassFlowRateTemperatureBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOUA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("m_dot") = m_dot_in;
    std::string nm = genName(name(), "rhouA_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "m_dot", "m_dot");
  }
  {
    std::string class_name = "OneDEnergyMassFlowRateTemperatureBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("m_dot") = m_dot_in;
    params.set<Real>("T") = T_in;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<MaterialPropertyName>("alpha") = FlowModel::UNITY;
    params.set<MaterialPropertyName>("p") = FlowModelSinglePhase::PRESSURE;
    params.set<std::vector<VariableName>>("arhoA") = cv_rhoA;
    params.set<std::vector<VariableName>>("arhouA") = cv_rhouA;
    params.set<std::vector<VariableName>>("arhoEA") = cv_rhoEA;
    params.set<UserObjectName>("fp") = _fp_name;
    std::string nm = genName(name(), "rhoEA_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "m_dot", "m_dot");
    connectObject(params, nm, "T", "T");
  }
}

void
InletMassFlowRateTemperature::setup1PhaseRDG()
{
  ExecFlagEnum userobject_execute_on(MooseUtils::getDefaultExecFlagEnum());
  userobject_execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  // boundary flux user object
  const std::string boundary_flux_name = genName(name(), "boundary_flux");
  {
    const std::string class_name = "BoundaryFlux3EqnGhostMassFlowRateTemperature";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<Real>("mass_flow_rate") = getParam<Real>("m_dot");
    params.set<Real>("T") = getParam<Real>("T");
    params.set<UserObjectName>("numerical_flux") = _numerical_flux_name;
    params.set<UserObjectName>("fluid_properties") = _fp_name;
    params.set<ExecFlagEnum>("execute_on") = userobject_execute_on;
    _sim.addUserObject(class_name, boundary_flux_name, params);
    connectObject(params, boundary_flux_name, "m_dot", "mass_flow_rate");
    connectObject(params, boundary_flux_name, "T", "T");
  }

  // BCs
  addWeakBC3Eqn(boundary_flux_name);
}

void
InletMassFlowRateTemperature::setup2PhaseCG()
{
  // coupling vectors
  std::vector<VariableName> cv_area(1, FlowModel::AREA);
  std::vector<VariableName> cv_beta(1, FlowModelTwoPhase::BETA);
  std::vector<VariableName> cv_alpha_liquid(1, FlowModelTwoPhase::VOLUME_FRACTION_LIQUID);
  std::vector<VariableName> cv_arhoA_liquid(1, FlowModelTwoPhase::ALPHA_RHO_A_LIQUID);
  std::vector<VariableName> cv_arhouA_liquid(1, FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID);
  std::vector<VariableName> cv_arhoEA_liquid(1, FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID);
  std::vector<VariableName> cv_pressure_liquid(1, FlowModelTwoPhase::PRESSURE_LIQUID);
  std::vector<VariableName> cv_alpha_vapor(1, FlowModelTwoPhase::VOLUME_FRACTION_VAPOR);
  std::vector<VariableName> cv_arhoA_vapor(1, FlowModelTwoPhase::ALPHA_RHO_A_VAPOR);
  std::vector<VariableName> cv_arhouA_vapor(1, FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR);
  std::vector<VariableName> cv_arhoEA_vapor(1, FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR);
  std::vector<VariableName> cv_pressure_vapor(1, FlowModelTwoPhase::PRESSURE_VAPOR);

  Real alpha = getParam<Real>("alpha_vapor");
  Real m_dot_liquid = getParam<Real>("m_dot_liquid");
  Real m_dot_vapor = getParam<Real>("m_dot_vapor");
  Real T_liquid = getParam<Real>("T_liquid");
  Real T_vapor = getParam<Real>("T_vapor");
  const TwoPhaseFluidProperties & tpfp = _sim.getUserObjectTempl<TwoPhaseFluidProperties>(_fp_name);
  const UserObjectName & fp_liquid = tpfp.getLiquidName();
  const UserObjectName & fp_vapor = tpfp.getVaporName();

  const FlowModelTwoPhase & fm = dynamic_cast<const FlowModelTwoPhase &>(*_flow_model);
  bool phase_interaction = fm.getPhaseInteraction();

  // BC for beta
  if (phase_interaction)
  {
    std::string class_name = "OneDVolumeFractionVaporBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::BETA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<UserObjectName>("vfm") = FlowModelTwoPhase::VOLUME_FRACTION_MAPPER;
    params.set<Real>("alpha") = alpha;
    std::string nm = genName(name(), "vf_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "alpha_vapor", "alpha");
  }

  // BC for liquid phase: mass, momentum, and energy
  {
    std::string class_name = "OneDMassMassFlowRateTemperatureBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHO_A_LIQUID;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("m_dot") = m_dot_liquid;
    std::string nm = genName(name(), "arhoA_liquid_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "m_dot_liquid", "m_dot");
  }
  {
    std::string class_name = "OneDMomentumMassFlowRateTemperatureBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("m_dot") = m_dot_liquid;
    std::string nm = genName(name(), "arhouA_liquid_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "m_dot_liquid", "m_dot");
  }
  {
    std::string class_name = "OneDEnergyMassFlowRateTemperatureBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("m_dot") = m_dot_liquid;
    params.set<Real>("T") = T_liquid;
    params.set<MaterialPropertyName>("alpha") = FlowModelTwoPhase::VOLUME_FRACTION_LIQUID;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<MaterialPropertyName>("p") = FlowModelTwoPhase::PRESSURE_LIQUID;
    params.set<std::vector<VariableName>>("arhoA") = cv_arhoA_liquid;
    params.set<std::vector<VariableName>>("arhouA") = cv_arhouA_liquid;
    params.set<std::vector<VariableName>>("arhoEA") = cv_arhoEA_liquid;
    params.set<std::vector<VariableName>>("beta") = cv_beta;
    params.set<UserObjectName>("fp") = fp_liquid;
    std::string nm = genName(name(), "arhoEA_liquid_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "m_dot_liquid", "m_dot");
    connectObject(params, nm, "T_liquid", "T");
  }

  // BC for vapor phase: mass, momentum, and energy
  {
    std::string class_name = "OneDMassMassFlowRateTemperatureBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHO_A_VAPOR;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("m_dot") = m_dot_vapor;
    std::string nm = genName(name(), "arhoA_vapor_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "m_dot_vapor", "m_dot");
  }
  {
    std::string class_name = "OneDMomentumMassFlowRateTemperatureBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("m_dot") = m_dot_vapor;
    std::string nm = genName(name(), "arhouA_vapor_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "m_dot_vapor", "m_dot");
  }
  {
    std::string class_name = "OneDEnergyMassFlowRateTemperatureBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("m_dot") = m_dot_vapor;
    params.set<Real>("T") = T_vapor;
    params.set<MaterialPropertyName>("alpha") = FlowModelTwoPhase::VOLUME_FRACTION_VAPOR;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<MaterialPropertyName>("p") = FlowModelTwoPhase::PRESSURE_VAPOR;
    params.set<std::vector<VariableName>>("arhoA") = cv_arhoA_vapor;
    params.set<std::vector<VariableName>>("arhouA") = cv_arhouA_vapor;
    params.set<std::vector<VariableName>>("arhoEA") = cv_arhoEA_vapor;
    params.set<std::vector<VariableName>>("beta") = cv_beta;
    params.set<UserObjectName>("fp") = fp_vapor;
    std::string nm = genName(name(), "arhoEA_vapor_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "m_dot_vapor", "m_dot");
    connectObject(params, nm, "T_vapor", "T");
  }
}

void
InletMassFlowRateTemperature::setup2PhaseRDG()
{
  ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
  execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  // boundary flux user object
  const std::string boundary_flux_name = genName(name(), "boundary_flux");
  {
    const std::string class_name = "BoundaryFlux7EqnGhostMassFlowRateTemperature";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<Real>("alpha_vapor") = getParam<Real>("alpha_vapor");
    params.set<Real>("m_dot_liquid") = getParam<Real>("m_dot_liquid");
    params.set<Real>("m_dot_vapor") = getParam<Real>("m_dot_vapor");
    params.set<Real>("T_liquid") = getParam<Real>("T_liquid");
    params.set<Real>("T_vapor") = getParam<Real>("T_vapor");
    params.set<UserObjectName>("fp_2phase") = _fp_name;
    params.set<UserObjectName>("numerical_flux") = _numerical_flux_name;
    params.set<UserObjectName>("vfm") = FlowModelTwoPhase::VOLUME_FRACTION_MAPPER;
    params.set<ExecFlagEnum>("execute_on") = execute_on;
    _sim.addUserObject(class_name, boundary_flux_name, params);

    connectObject(params, boundary_flux_name, "alpha_vapor");
    connectObject(params, boundary_flux_name, "m_dot_liquid");
    connectObject(params, boundary_flux_name, "m_dot_vapor");
    connectObject(params, boundary_flux_name, "T_liquid");
    connectObject(params, boundary_flux_name, "T_vapor");
  }

  // BCs
  addWeakBC7Eqn(boundary_flux_name);
}

void
InletMassFlowRateTemperature::setup2PhaseNCGCG()
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
InletMassFlowRateTemperature::addMooseObjects()
{
  // apply logic for parameters with one- and two-phase variants
  const bool is_two_phase =
      _flow_model_id == THM::FM_TWO_PHASE || _flow_model_id == THM::FM_TWO_PHASE_NCG;
  getOneOrTwoPhaseParameters<Real>(is_two_phase, "m_dot", {"m_dot_liquid", "m_dot_vapor"}, *this);
  getOneOrTwoPhaseParameters<Real>(is_two_phase, "T", {"T_liquid", "T_vapor"}, *this);

  if (_flow_model_id == THM::FM_SINGLE_PHASE)
  {
    if (_spatial_discretization == FlowModel::CG)
      setup1PhaseCG();
    else if (_spatial_discretization == FlowModel::rDG)
      setup1PhaseRDG();
  }
  else if (_flow_model_id == THM::FM_TWO_PHASE)
  {
    if (_spatial_discretization == FlowModel::CG)
      setup2PhaseCG();
    else if (_spatial_discretization == FlowModel::rDG)
      setup2PhaseRDG();
  }
  else if (_flow_model_id == THM::FM_TWO_PHASE_NCG)
    setup2PhaseNCGCG();
}
