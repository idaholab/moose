#include "InletDensityVelocity1Phase.h"
#include "FlowModelSinglePhase.h"

registerMooseObject("THMApp", InletDensityVelocity1Phase);

template <>
InputParameters
validParams<InletDensityVelocity1Phase>()
{
  InputParameters params = validParams<FlowBoundary>();
  params.addRequiredParam<Real>("rho", "Prescribed density [kg/m^3]");
  params.addRequiredParam<Real>("vel", "Prescribed velocity [m/s]");
  params.addParam<bool>("reversible", false, "true for reversible, false (default) for pure inlet");
  params.addClassDescription(
      "Boundary condition with prescribed density and velocity for 1-phase flow channels.");
  return params;
}

InletDensityVelocity1Phase::InletDensityVelocity1Phase(const InputParameters & params)
  : FlowBoundary(params), _reversible(getParam<bool>("reversible"))
{
}

void
InletDensityVelocity1Phase::check() const
{
  FlowBoundary::check();

  auto fm = dynamic_cast<const FlowModelSinglePhase *>(_flow_model.get());
  if (fm == nullptr)
    logError("Incompatible flow model. Make sure you use this component with single phase flow "
             "channel.");

  if (_spatial_discretization == FlowModel::rDG)
    logSpatialDiscretizationNotImplementedError(_spatial_discretization);
}

void
InletDensityVelocity1Phase::addMooseObjects()
{
  Real rho_in = getParam<Real>("rho");
  Real u_in = getParam<Real>("vel");

  {
    std::string class_name = "OneDAreaTimesConstantBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("value") = rho_in;
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    std::string nm = genName(name(), "rhoA_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "rho", "value");
  }
  {
    std::string class_name = "OneDMomentumDensityVelocityBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOUA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("rho") = rho_in;
    params.set<Real>("vel") = u_in;
    params.set<UserObjectName>("fp") = _fp_name;
    params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<MaterialPropertyName>("alpha") = FlowModel::UNITY;
    std::string nm = genName(name(), "rhouA_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "rho");
    connectObject(params, nm, "vel");
  }
  {
    std::string class_name = "OneDEnergyDensityVelocityBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("rho") = rho_in;
    params.set<Real>("vel") = u_in;
    params.set<MaterialPropertyName>("alpha") = FlowModel::UNITY;
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
    params.set<UserObjectName>("fp") = _fp_name;
    std::string nm = genName(name(), "rhoEA_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "rho");
    connectObject(params, nm, "vel");
  }
}
