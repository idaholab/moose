#include "InletStagnationPressureTemperature1Phase.h"
#include "FlowModelSinglePhase.h"

registerMooseObject("THMApp", InletStagnationPressureTemperature1Phase);

template <>
InputParameters
validParams<InletStagnationPressureTemperature1Phase>()
{
  InputParameters params = validParams<FlowBoundary>();
  params.addRequiredParam<Real>("p0", "Prescribed stagnation pressure");
  params.addRequiredParam<Real>("T0", "Prescribed stagnation temperature");
  params.addParam<bool>("reversible", false, "true for reversible, false (default) for pure inlet");
  params.addClassDescription("Boundary condition with prescribed stagnation pressure and "
                             "temperature for 1-phase flow channels.");
  return params;
}

InletStagnationPressureTemperature1Phase::InletStagnationPressureTemperature1Phase(
    const InputParameters & params)
  : FlowBoundary(params), _reversible(getParam<bool>("reversible"))
{
}

void
InletStagnationPressureTemperature1Phase::check() const
{
  FlowBoundary::check();

  auto fm = dynamic_cast<const FlowModelSinglePhase *>(_flow_model.get());
  if (fm == nullptr)
    logError("Incompatible flow model. Make sure you use this component with single phase flow "
             "channel.");

  if ((_spatial_discretization == FlowModel::rDG) && (_flow_model_id == THM::FM_TWO_PHASE_NCG))
    logSpatialDiscretizationNotImplementedError(_spatial_discretization);
}

void
InletStagnationPressureTemperature1Phase::setup1PhaseCG()
{
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
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<std::vector<VariableName>>("vel") = {FlowModelSinglePhase::VELOCITY};
    params.set<std::vector<VariableName>>("arhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("arhouA") = {FlowModelSinglePhase::RHOUA};
    params.set<std::vector<VariableName>>("arhoEA") = {FlowModelSinglePhase::RHOEA};
    std::string nm = genName(name(), "rhoA_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "p0");
    connectObject(params, nm, "T0");
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
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<std::vector<VariableName>>("vel") = {FlowModelSinglePhase::VELOCITY};
    params.set<MaterialPropertyName>("alpha") = FlowModel::UNITY;
    params.set<std::vector<VariableName>>("arhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("arhouA") = {FlowModelSinglePhase::RHOUA};
    params.set<std::vector<VariableName>>("arhoEA") = {FlowModelSinglePhase::RHOEA};
    std::string nm = genName(name(), "rhouA_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "p0");
    connectObject(params, nm, "T0");
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
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<std::vector<VariableName>>("vel") = {FlowModelSinglePhase::VELOCITY};
    params.set<std::vector<VariableName>>("arhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("arhouA") = {FlowModelSinglePhase::RHOUA};
    params.set<std::vector<VariableName>>("arhoEA") = {FlowModelSinglePhase::RHOEA};
    std::string nm = genName(name(), "rhoEA_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "p0");
    connectObject(params, nm, "T0");
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
      params.set<std::vector<VariableName>>("arhouA") = {FlowModelSinglePhase::RHOUA};
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
      params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
      params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
      params.set<MaterialPropertyName>("alpha") = FlowModel::UNITY;
      params.set<std::vector<VariableName>>("vel") = {FlowModelSinglePhase::VELOCITY};
      params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
      std::string nm = genName(name(), "bc_rhou", "rev");
      _sim.addBoundaryCondition(class_name, nm, params);
      connectObject(params, nm, "p0", "p_in");
    }
    {
      std::string class_name = "OneDEnergyStaticPressureBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
      params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
      params.set<UserObjectName>("fp") = _fp_name;
      params.set<bool>("is_liquid") = true;
      params.set<Real>("p_in") = p0;
      params.set<bool>("reversible") = _reversible;
      params.set<Real>("normal") = _normal;
      // coupling
      params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
      params.set<std::vector<VariableName>>("rho") = {FlowModelSinglePhase::DENSITY};
      params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
      params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
      params.set<std::vector<VariableName>>("vel") = {FlowModelSinglePhase::VELOCITY};
      params.set<std::vector<VariableName>>("e") = {FlowModelSinglePhase::SPECIFIC_INTERNAL_ENERGY};
      params.set<std::vector<VariableName>>("v") = {FlowModelSinglePhase::SPECIFIC_VOLUME};
      std::string nm = genName(name(), "bc_rhoE", "rev");
      _sim.addBoundaryCondition(class_name, nm, params);
      connectObject(params, nm, "p0", "p_in");
    }
  }
}

void
InletStagnationPressureTemperature1Phase::setup1PhaseRDG()
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
    connectObject(params, boundary_flux_name, "p0");
    connectObject(params, boundary_flux_name, "T0");
  }

  // BCs
  addWeakBC3Eqn(boundary_flux_name);
}

void
InletStagnationPressureTemperature1Phase::addMooseObjects()
{
  if (_spatial_discretization == FlowModel::CG)
    setup1PhaseCG();
  else if (_spatial_discretization == FlowModel::rDG)
    setup1PhaseRDG();
}
