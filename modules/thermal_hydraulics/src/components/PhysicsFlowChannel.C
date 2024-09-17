//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhysicsFlowChannel.h"
#include "SinglePhaseFluidProperties.h"
#include "PhysicsHeatTransferBase.h"
#include "ThermalHydraulicsApp.h"

registerMooseObject("ThermalHydraulicsApp", PhysicsFlowChannel);

InputParameters
PhysicsFlowChannel::validParams()
{
  InputParameters params = FlowChannelBase::validParams();

  params.addParam<FunctionName>("initial_p", "Initial pressure in the flow channel [Pa]");
  params.addParam<FunctionName>("initial_vel", "Initial velocity in the flow channel [m/s]");
  params.addParam<FunctionName>("initial_T", "Initial temperature in the flow channel [K]");
  params.addParam<FunctionName>("D_h", "Hydraulic diameter [m]");

  params.addRequiredParam<std::vector<PhysicsName>>("physics",
                                                    "Physics active on the flow channel");

  params.declareControllable("initial_p initial_T initial_vel D_h");
  params.addParamNamesToGroup("initial_p initial_T initial_vel", "Variable initialization");
  params.addClassDescription("1D flow channel using Physics to define the equations");

  return params;
}

PhysicsFlowChannel::PhysicsFlowChannel(const InputParameters & params) : FlowChannelBase(params) {}

void
PhysicsFlowChannel::init()
{
  FlowChannelBase::init();

  const UserObject & fp = getTHMProblem().getUserObject<UserObject>(_fp_name);
  if (dynamic_cast<const SinglePhaseFluidProperties *>(&fp) == nullptr)
    logError("Supplied fluid properties must be for 1-phase fluids.");

  for (const auto & physics_name : getParam<std::vector<PhysicsName>>("physics"))
    _th_physics.insert(
        _app.actionWarehouse().getPhysics<ThermalHydraulicsFlowPhysics>(physics_name));
  // NOTE: we currently expect to error on non thermal-hydraulics physics.
  // This may be removed in the future

  for (auto th_phys : _th_physics)
    th_phys->addFlowChannel(this);

  // This has to be early
  addDirectionVariables();
}

void
PhysicsFlowChannel::check() const
{
  FlowChannelBase::check();

  // only 1-phase flow compatible heat transfers are allowed
  for (unsigned int i = 0; i < _heat_transfer_names.size(); i++)
  {
    if (!hasComponentByName<PhysicsHeatTransferBase>(_heat_transfer_names[i]))
      logError("Coupled heat source '",
               _heat_transfer_names[i],
               "' is not compatible with Physics flow channel. Use a Physics heat transfer "
               "component instead.");
  }

  bool ics_set =
      getTHMProblem().hasInitialConditionsFromFile() ||
      (isParamValid("initial_p") && isParamValid("initial_T") && isParamValid("initial_vel"));

  if (!ics_set && !_app.isRestarting())
  {
    // create a list of the missing IC parameters
    const std::vector<std::string> ic_params{"initial_p", "initial_T", "initial_vel"};
    std::ostringstream oss;
    for (const auto & ic_param : ic_params)
      if (!isParamValid(ic_param))
        oss << " " << ic_param;

    logError("The following initial condition parameters are missing:", oss.str());
  }
}

void
PhysicsFlowChannel::addMooseObjects()
{
  FlowChannelBase::addMooseObjects();

  if (!_pipe_pars_transferred)
    addHydraulicDiameterMaterial();
}

void
PhysicsFlowChannel::addHydraulicDiameterMaterial()
{
  const std::string mat_name = genName(name(), "D_h_material");

  if (isParamValid("D_h"))
  {
    const FunctionName & D_h_fn_name = getParam<FunctionName>("D_h");

    const std::string class_name = "ADGenericFunctionMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
    params.set<std::vector<std::string>>("prop_names") = {FlowModelSinglePhase::HYDRAULIC_DIAMETER};
    params.set<std::vector<FunctionName>>("prop_values") = {D_h_fn_name};
    getTHMProblem().addMaterial(class_name, mat_name, params);

    makeFunctionControllableIfConstant(D_h_fn_name, "D_h");
  }
  else
  {
    const std::string class_name = "ADHydraulicDiameterCircularMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
    params.set<MaterialPropertyName>("D_h_name") = FlowModelSinglePhase::HYDRAULIC_DIAMETER;
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    getTHMProblem().addMaterial(class_name, mat_name, params);
  }
}

void
PhysicsFlowChannel::addDirectionVariables()
{
  std::vector<std::string> channel_direction = {"direction_x", "direction_y", "direction_z"};
  const auto dimension = 3;

  // Add a variable (done in every flow channel)
  {
    const auto variable_type = "MooseVariableFVReal";
    auto params = _factory.getValidParams(variable_type);
    params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();

    for (const auto d : make_range(dimension))
      getTHMProblem().addSimVariable(false, variable_type, channel_direction[d], params);
  }

  // Add an IC
  {
    const auto ic_type = "ConstantIC";
    auto params = _factory.getValidParams(ic_type);
    params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();

    for (const auto d : make_range(dimension))
    {
      params.set<VariableName>("variable") = channel_direction[d];
      params.set<Real>("value") = getParam<RealVectorValue>("orientation")(d);
      getTHMProblem().addSimInitialCondition(
          ic_type, genName(name(), channel_direction[d]), params);
    }
  }
}

void
PhysicsFlowChannel::getHeatTransferVariableNames()
{
  FlowChannelBase::getHeatTransferVariableNames();

  for (unsigned int i = 0; i < _n_heat_transfer_connections; i++)
  {
    const PhysicsHeatTransferBase & heat_transfer =
        getComponentByName<PhysicsHeatTransferBase>(_heat_transfer_names[i]);

    _Hw_names.push_back(heat_transfer.getWallHeatTransferCoefficientName());
  }
}
