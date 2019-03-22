#include "HeatTransferFromExternalAppTemperature1Phase.h"

registerMooseObject("THMApp", HeatTransferFromExternalAppTemperature1Phase);

template <>
InputParameters
validParams<HeatTransferFromExternalAppTemperature1Phase>()
{
  InputParameters params = validParams<HeatTransferFromTemperature1Phase>();
  params.addParam<FunctionName>("initial_T_wall", "Initial condition for wall temperature");
  params.addClassDescription("Heat transfer into 1-phase flow channel from temperature provided by "
                             "an external application");
  return params;
}

HeatTransferFromExternalAppTemperature1Phase::HeatTransferFromExternalAppTemperature1Phase(
    const InputParameters & parameters)
  : HeatTransferFromTemperature1Phase(parameters)
{
}

void
HeatTransferFromExternalAppTemperature1Phase::check() const
{
  HeatTransferFromTemperature1Phase::check();

  if (!isParamValid("initial_T_wall") && !_app.isRestarting())
    logError("Missing initial condition for wall temperature.");
}

void
HeatTransferFromExternalAppTemperature1Phase::addVariables()
{
  HeatTransferFromTemperature1Phase::addVariables();

  _sim.addFunctionIC(_T_wall_name, getParam<FunctionName>("initial_T_wall"), _pipe_name);
}

void
HeatTransferFromExternalAppTemperature1Phase::addMooseObjects()
{
  HeatTransferFromTemperature1Phase::addMooseObjects();

  addHeatTransferKernels();
}
