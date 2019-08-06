#include "InletMassFlowRateTemperature1Phase.h"
#include "FlowModelSinglePhase.h"

registerMooseObject("THMApp", InletMassFlowRateTemperature1Phase);

template <>
InputParameters
validParams<InletMassFlowRateTemperature1Phase>()
{
  InputParameters params = validParams<FlowBoundary>();
  params.addRequiredParam<Real>("m_dot", "Prescribed mass flow rate");
  params.addRequiredParam<Real>("T", "prescribed temperature (used only in 3eqn model)");
  params.addParam<bool>("reversible", false, "true for reversible, false (default) for pure inlet");
  params.addClassDescription("Boundary condition with prescribed mass flow rate and temperature "
                             "for 1-phase flow channels.");
  return params;
}

InletMassFlowRateTemperature1Phase::InletMassFlowRateTemperature1Phase(
    const InputParameters & params)
  : FlowBoundary(params), _reversible(getParam<bool>("reversible"))
{
}

void
InletMassFlowRateTemperature1Phase::check() const
{
  FlowBoundary::check();

  auto fm = dynamic_cast<const FlowModelSinglePhase *>(_flow_model.get());
  if (fm == nullptr)
    logError("Incompatible flow model. Make sure you use this component with single phase flow "
             "channel.");
}

void
InletMassFlowRateTemperature1Phase::setup1PhaseCG()
{
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
    connectObject(params, nm, "m_dot");
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
    connectObject(params, nm, "m_dot");
  }
  {
    std::string class_name = "OneDEnergyMassFlowRateTemperatureBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("m_dot") = m_dot_in;
    params.set<Real>("T") = T_in;
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<MaterialPropertyName>("alpha") = FlowModel::UNITY;
    params.set<MaterialPropertyName>("p") = FlowModelSinglePhase::PRESSURE;
    params.set<std::vector<VariableName>>("arhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("arhouA") = {FlowModelSinglePhase::RHOUA};
    params.set<std::vector<VariableName>>("arhoEA") = {FlowModelSinglePhase::RHOEA};
    params.set<UserObjectName>("fp") = _fp_name;
    std::string nm = genName(name(), "rhoEA_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "m_dot");
    connectObject(params, nm, "T");
  }
}

void
InletMassFlowRateTemperature1Phase::setup1PhaseRDG()
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
    connectObject(params, boundary_flux_name, "T");
  }

  // BCs
  addWeakBC3Eqn(boundary_flux_name);
}

void
InletMassFlowRateTemperature1Phase::addMooseObjects()
{
  if (_spatial_discretization == FlowModel::CG)
    setup1PhaseCG();
  else if (_spatial_discretization == FlowModel::rDG)
    setup1PhaseRDG();
}
