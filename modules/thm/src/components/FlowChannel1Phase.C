#include "FlowChannel1Phase.h"
#include "FlowModelSinglePhase.h"
#include "StabilizationSettings.h"
#include "HeatTransfer1PhaseBase.h"
#include "Closures1PhaseBase.h"
#include "THMApp.h"

registerMooseObject("THMApp", FlowChannel1Phase);

template <>
InputParameters
validParams<FlowChannel1Phase>()
{
  InputParameters params = validParams<FlowChannelBase>();
  params.addRequiredParam<FunctionName>(
      "A", "Area of the flow channel, can be a constant or a function");
  params.addParam<Real>("roughness", 0.0, "roughness, [m]");
  params.addParam<FunctionName>("f", "Wall friction");

  params.addParam<MooseEnum>("heat_transfer_geom",
                             FlowChannel1Phase::getConvHeatTransGeometry("PIPE"),
                             "Convective heat transfer geometry");
  params.addParam<Real>("PoD", 1, "pitch to diameter ratio for parallel bundle heat transfer");

  params.addParam<FunctionName>("initial_p", "Initial pressure in the flow channel");
  params.addParam<FunctionName>("initial_vel", "Initial velocity in the flow channel");
  params.addParam<FunctionName>("initial_T", "Initial temperature in the flow channel");

  params.addClassDescription("1-phase 1D flow channel");

  return params;
}

FlowChannel1Phase::FlowChannel1Phase(const InputParameters & params) : FlowChannelBase(params) {}

std::shared_ptr<FlowModel>
FlowChannel1Phase::buildFlowModel()
{
  const std::string class_name = "FlowModelSinglePhase";
  InputParameters pars = _factory.getValidParams(class_name);
  pars.set<Simulation *>("_sim") = &_sim;
  pars.set<FlowChannelBase *>("_flow_channel") = this;
  pars.set<UserObjectName>("fp") = _fp_name;
  pars.set<UserObjectName>("numerical_flux") = _numerical_flux_name;
  pars.set<AuxVariableName>("A_linear_name") = _A_linear_name;
  pars.set<MooseEnum>("rdg_slope_reconstruction") = _rdg_slope_reconstruction;
  return _factory.create<FlowModel>(class_name, name(), pars, 0);
}

void
FlowChannel1Phase::check() const
{
  FlowChannelBase::check();

  // only 1-phase flow compatible heat transfers are allowed
  for (unsigned int i = 0; i < _heat_transfer_names.size(); i++)
  {
    if (!hasComponentByName<HeatTransfer1PhaseBase>(_heat_transfer_names[i]))
      logError("Coupled heat sources '",
               _heat_transfer_names[i],
               "' is not compatible with single phase flow channel. Either change the type of the "
               "flow channel or the heat transfer component.");
  }

  bool ics_set =
      _sim.hasInitialConditionsFromFile() ||
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
FlowChannel1Phase::getHeatTransferVariableNames()
{
  FlowChannelBase::getHeatTransferVariableNames();

  for (unsigned int i = 0; i < _n_heat_transfer_connections; i++)
  {
    const HeatTransfer1PhaseBase & heat_transfer =
        getComponentByName<HeatTransfer1PhaseBase>(_heat_transfer_names[i]);

    _Hw_1phase_names.push_back(heat_transfer.getWallHeatTransferCoefficient1PhaseName());
  }
}
