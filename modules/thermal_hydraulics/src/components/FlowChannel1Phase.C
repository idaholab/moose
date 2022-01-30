#include "FlowChannel1Phase.h"
#include "FlowModelSinglePhase.h"
#include "SinglePhaseFluidProperties.h"
#include "HeatTransfer1PhaseBase.h"
#include "Closures1PhaseBase.h"
#include "ThermalHydraulicsApp.h"

registerMooseObject("ThermalHydraulicsApp", FlowChannel1Phase);

InputParameters
FlowChannel1Phase::validParams()
{
  InputParameters params = FlowChannelBase::validParams();

  params.addParam<FunctionName>("initial_p", "Initial pressure in the flow channel [Pa]");
  params.addParam<FunctionName>("initial_vel", "Initial velocity in the flow channel [m/s]");
  params.addParam<FunctionName>("initial_T", "Initial temperature in the flow channel [K]");

  params.addClassDescription("1-phase 1D flow channel");

  return params;
}

FlowChannel1Phase::FlowChannel1Phase(const InputParameters & params) : FlowChannelBase(params) {}

void
FlowChannel1Phase::init()
{
  FlowChannelBase::init();

  const UserObject & fp = _sim.getUserObject<UserObject>(_fp_name);
  if (dynamic_cast<const SinglePhaseFluidProperties *>(&fp) == nullptr)
    logError("Supplied fluid properties must be for 1-phase fluids.");
}

std::shared_ptr<FlowModel>
FlowChannel1Phase::buildFlowModel()
{
  const std::string class_name = "FlowModelSinglePhase";
  InputParameters pars = _factory.getValidParams(class_name);
  pars.set<THMProblem *>("_thm_problem") = &_sim;
  pars.set<FlowChannelBase *>("_flow_channel") = this;
  pars.set<UserObjectName>("fp") = _fp_name;
  pars.set<UserObjectName>("numerical_flux") = _numerical_flux_name;
  pars.set<MooseEnum>("rdg_slope_reconstruction") = _rdg_slope_reconstruction;
  pars.set<bool>("output_vector_velocity") = _sim.getVectorValuedVelocity();
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
      logError("Coupled heat source '",
               _heat_transfer_names[i],
               "' is not compatible with single phase flow channel. Use single phase heat transfer "
               "component instead.");
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
