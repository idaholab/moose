#include "Outlet1Phase.h"
#include "FlowModelSinglePhase.h"

registerMooseObject("THMApp", Outlet1Phase);

template <>
InputParameters
validParams<Outlet1Phase>()
{
  InputParameters params = validParams<FlowBoundary>();
  params.addParam<bool>("reversible", false, "true for reversible outlet boundary conditions");
  params.addRequiredParam<Real>("p", "Prescribed pressure [Pa]");
  params.addParam<bool>(
      "legacy", false, "Use the old version of the BC (violating characteristic theory) or not.");
  params.addClassDescription(
      "Boundary condition with prescribed pressure for 1-phase flow channels.");
  return params;
}

Outlet1Phase::Outlet1Phase(const InputParameters & params)
  : FlowBoundary(params),
    _reversible(getParam<bool>("reversible")),
    _legacy(getParam<bool>("legacy"))
{
}

void
Outlet1Phase::check() const
{
  FlowBoundary::check();

  auto fm = dynamic_cast<const FlowModelSinglePhase *>(_flow_model.get());
  if (fm == nullptr)
    logError("Incompatible flow model. Make sure you use this component with single phase flow "
             "channel.");
}

void
Outlet1Phase::add3EqnStaticPBC()
{
  Real p = getParam<Real>("p");

  {
    std::string class_name = "OneDMassFreeInletReverseBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOA;
    params.set<Real>("normal") = _normal;
    params.set<bool>("reversible") = _reversible;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<std::vector<VariableName>>("arhouA") = {FlowModelSinglePhase::RHOUA};
    _sim.addBoundaryCondition(class_name, genName(name(), "rhoA"), params);
  }

  {
    std::string class_name = "OneDMomentumStaticPressureBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOUA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<bool>("reversible") = _reversible;
    params.set<Real>("p_in") = p;
    // coupling
    params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
    params.set<MaterialPropertyName>("alpha") = FlowModel::UNITY;
    params.set<std::vector<VariableName>>("vel") = {FlowModelSinglePhase::VELOCITY};
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    std::string nm = genName(name(), "bc_rhou");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "p", "p_in");
  }

  {
    std::string class_name = "OneDEnergyStaticPressureBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<UserObjectName>("fp") = _fp_name;
    params.set<bool>("reversible") = _reversible;
    params.set<bool>("is_liquid") = true;
    params.set<Real>("p_in") = p;
    params.set<Real>("normal") = _normal;
    // coupling
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<std::vector<VariableName>>("rho") = {FlowModelSinglePhase::DENSITY};
    params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
    params.set<std::vector<VariableName>>("vel") = {FlowModelSinglePhase::VELOCITY};
    params.set<std::vector<VariableName>>("e") = {FlowModelSinglePhase::SPECIFIC_INTERNAL_ENERGY};
    params.set<std::vector<VariableName>>("v") = {FlowModelSinglePhase::SPECIFIC_VOLUME};
    std::string nm = genName(name(), "bc_rhoE");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "p", "p_in");
  }
  {
    std::string class_name = "OneDEnergyStaticPressureSupersonicBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<UserObjectName>("fp") = _fp_name;
    params.set<bool>("reversible") = _reversible;
    params.set<bool>("is_liquid") = true;
    // coupling
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<std::vector<VariableName>>("arhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("arhouA") = {FlowModelSinglePhase::RHOUA};
    params.set<std::vector<VariableName>>("arhoEA") = {FlowModelSinglePhase::RHOEA};
    params.set<std::vector<VariableName>>("H") = {FlowModelSinglePhase::SPECIFIC_TOTAL_ENTHALPY};
    params.set<std::vector<VariableName>>("vel") = {FlowModelSinglePhase::VELOCITY};
    params.set<std::vector<VariableName>>("e") = {FlowModelSinglePhase::SPECIFIC_INTERNAL_ENERGY};
    params.set<std::vector<VariableName>>("v") = {FlowModelSinglePhase::SPECIFIC_VOLUME};
    params.set<MaterialPropertyName>("p") = FlowModelSinglePhase::PRESSURE;
    params.set<MaterialPropertyName>("c") = FlowModelSinglePhase::SOUND_SPEED;
    std::string nm = genName(name(), "bc_rhoE2");
    _sim.addBoundaryCondition(class_name, nm, params);
  }
}

void
Outlet1Phase::add3EqnStaticPReverseBC()
{
  Real p = getParam<Real>("p");

  {
    std::string class_name = "OneD7EqnMassStaticPressureReverseBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("p") = p;
    params.set<Real>("normal") = _normal;
    params.set<UserObjectName>("fp") = _fp_name;
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<std::vector<VariableName>>("T") = {FlowModelSinglePhase::TEMPERATURE};
    params.set<std::vector<VariableName>>("vel") = {FlowModelSinglePhase::VELOCITY};
    std::string nm = genName(name(), "rhoA", "rev");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "p");
  }
  {
    std::string class_name = "OneDMomentumStaticPressureReverseBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOUA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("p") = p;
    params.set<UserObjectName>("fp") = _fp_name;
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<std::vector<VariableName>>("vel") = {FlowModelSinglePhase::VELOCITY};
    params.set<std::vector<VariableName>>("arhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("T") = {FlowModelSinglePhase::TEMPERATURE};
    std::string nm = genName(name(), "rhouA", "rev");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "p");
  }
  {
    std::string class_name = "OneD7EqnEnergyStaticPressureReverseBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("p") = p;
    params.set<Real>("normal") = _normal;
    params.set<UserObjectName>("fp") = _fp_name;
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<std::vector<VariableName>>("vel") = {FlowModelSinglePhase::VELOCITY};
    params.set<std::vector<VariableName>>("T") = {FlowModelSinglePhase::TEMPERATURE};
    std::string nm = genName(name(), "rhoEA", "rev");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "p");
  }
}

void
Outlet1Phase::add3EqnStaticPBCLegacy()
{
  Real p = getParam<Real>("p");

  {
    std::string class_name = "OneDMassFreeInletReverseBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<bool>("reversible") = _reversible;
    params.set<std::vector<VariableName>>("arhouA") = {FlowModelSinglePhase::RHOUA};
    _sim.addBoundaryCondition(class_name, genName(name(), "rhoA"), params);
  }

  {
    std::string class_name = "OneDMomentumStaticPressureLegacyBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOUA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<bool>("reversible") = _reversible;
    params.set<Real>("p_in") = p;
    // coupling
    params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<std::vector<VariableName>>("vel") = {FlowModelSinglePhase::VELOCITY};
    params.set<MaterialPropertyName>("alpha") = FlowModel::UNITY;
    std::string nm = genName(name(), "bc_rhou");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "p", "p_in");
  }

  {
    std::string class_name = "OneDEnergyStaticPressureLegacyBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<bool>("reversible") = _reversible;
    params.set<Real>("p_in") = p;
    // coupling
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<std::vector<VariableName>>("arhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("arhouA") = {FlowModelSinglePhase::RHOUA};
    params.set<std::vector<VariableName>>("vel") = {FlowModelSinglePhase::VELOCITY};
    params.set<MaterialPropertyName>("alpha") = FlowModel::UNITY;
    std::string nm = genName(name(), "bc_rhoE");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "p", "p_in");
  }
}

void
Outlet1Phase::addMooseObjects3EqnRDG()
{
  ExecFlagEnum userobject_execute_on(MooseUtils::getDefaultExecFlagEnum());
  userobject_execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  // boundary flux user object
  const std::string boundary_flux_name = genName(name(), "boundary_flux");
  {
    const std::string class_name = "BoundaryFlux3EqnGhostPressure";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<Real>("p") = getParam<Real>("p");
    params.set<UserObjectName>("fluid_properties") = _fp_name;
    params.set<UserObjectName>("numerical_flux") = _numerical_flux_name;
    params.set<ExecFlagEnum>("execute_on") = userobject_execute_on;
    _sim.addUserObject(class_name, boundary_flux_name, params);
    connectObject(params, boundary_flux_name, "p");
  }

  // BCs
  addWeakBC3Eqn(boundary_flux_name);
}

void
Outlet1Phase::addMooseObjects()
{
  if (_spatial_discretization == FlowModel::CG)
  {
    if (_legacy)
      add3EqnStaticPBCLegacy();
    else
      add3EqnStaticPBC();
    if (_reversible)
      add3EqnStaticPReverseBC();
  }
  else if (_spatial_discretization == FlowModel::rDG)
    addMooseObjects3EqnRDG();
}
