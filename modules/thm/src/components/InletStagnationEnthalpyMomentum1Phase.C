#include "InletStagnationEnthalpyMomentum1Phase.h"
#include "FlowModelSinglePhase.h"

registerMooseObject("THMApp", InletStagnationEnthalpyMomentum1Phase);

template <>
InputParameters
validParams<InletStagnationEnthalpyMomentum1Phase>()
{
  InputParameters params = validParams<FlowBoundary>();
  params.addRequiredParam<Real>("rhou", "Prescribed momentum");
  params.addRequiredParam<Real>("H", "Prescribed specific total enthalpy");
  params.addParam<bool>("reversible", false, "true for reversible, false (default) for pure inlet");
  params.addClassDescription("Boundary condition with prescribed stagnation enthalpy and momentum "
                             "for 1-phase flow channels.");
  return params;
}

InletStagnationEnthalpyMomentum1Phase::InletStagnationEnthalpyMomentum1Phase(
    const InputParameters & params)
  : FlowBoundary(params), _reversible(getParam<bool>("reversible"))
{
}

void
InletStagnationEnthalpyMomentum1Phase::check() const
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
InletStagnationEnthalpyMomentum1Phase::addMooseObjects()
{
  Real H = getParam<Real>("H");
  Real rhou = getParam<Real>("rhou");

  {
    std::string class_name = "OneDMassHRhoUBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("rhou") = rhou;
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<MaterialPropertyName>("alpha") = FlowModel::UNITY;
    std::string nm = genName(name(), "rhoA_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "rhou");
  }

  {
    std::string class_name = "OneDMomentumHRhoUBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOUA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("rhou") = rhou;
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<std::vector<VariableName>>("arhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("arhouA") = {FlowModelSinglePhase::RHOUA};
    params.set<std::vector<VariableName>>("arhoEA") = {FlowModelSinglePhase::RHOEA};
    params.set<MaterialPropertyName>("p") = FlowModelSinglePhase::PRESSURE;
    std::string nm = genName(name(), "rhouA_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "rhou");
  }

  {
    std::string class_name = "OneDEnergyHRhoUBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
    params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
    params.set<Real>("normal") = _normal;
    params.set<Real>("rhou") = rhou;
    params.set<Real>("H") = H;
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<MaterialPropertyName>("alpha") = FlowModel::UNITY;
    std::string nm = genName(name(), "rhoEA_bc");
    _sim.addBoundaryCondition(class_name, nm, params);
    connectObject(params, nm, "rhou");
    connectObject(params, nm, "H");
  }
}
