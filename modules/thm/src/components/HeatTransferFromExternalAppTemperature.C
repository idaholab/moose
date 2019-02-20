#include "HeatTransferFromExternalAppTemperature.h"

registerMooseObject("THMApp", HeatTransferFromExternalAppTemperature);

template <>
InputParameters
validParams<HeatTransferFromExternalAppTemperature>()
{
  InputParameters params = validParams<HeatTransferFromTemperature>();
  params.addParam<FunctionName>("initial_T_wall", "Initial condition for wall temperature");
  return params;
}

HeatTransferFromExternalAppTemperature::HeatTransferFromExternalAppTemperature(
    const InputParameters & parameters)
  : HeatTransferFromTemperature(parameters)
{
}

void
HeatTransferFromExternalAppTemperature::check() const
{
  HeatTransferFromTemperature::check();

  if (!isParamValid("initial_T_wall") && !_app.isRestarting())
    logError("Missing initial condition for wall temperature.");
}

void
HeatTransferFromExternalAppTemperature::addVariables()
{
  HeatTransferFromTemperature::addVariables();

  _sim.addFunctionIC(_T_wall_name, getParam<FunctionName>("initial_T_wall"), _pipe_name);
}

void
HeatTransferFromExternalAppTemperature::addMooseObjects()
{
  HeatTransferFromTemperature::addMooseObjects();

  if (_model_type == THM::FM_SINGLE_PHASE)
    addHeatTransferKernels1Phase();
  else if (_model_type == THM::FM_TWO_PHASE || _model_type == THM::FM_TWO_PHASE_NCG)
    addHeatTransferKernels2Phase();
}
