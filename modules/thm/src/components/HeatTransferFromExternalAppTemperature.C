#include "HeatTransferFromExternalAppTemperature.h"

registerMooseObject("THMApp", HeatTransferFromExternalAppTemperature);

template <>
InputParameters
validParams<HeatTransferFromExternalAppTemperature>()
{
  InputParameters params = validParams<HeatTransferBase>();
  params.addParam<FunctionName>("initial_T_wall", "Initial condition for wall temperature");
  return params;
}

HeatTransferFromExternalAppTemperature::HeatTransferFromExternalAppTemperature(
    const InputParameters & parameters)
  : HeatTransferBase(parameters)
{
}

void
HeatTransferFromExternalAppTemperature::check() const
{
  HeatTransferBase::check();

  if (_model_type == THM::FM_SINGLE_PHASE)
    logError("HeatTransferFromExternalAppTemperature component is deprecated. Use 'type = "
             "HeatTransferFromExternalAppTemperature1Phase' instead.");
  else if (_model_type == THM::FM_TWO_PHASE || _model_type == THM::FM_TWO_PHASE_NCG)
    logError("HeatTransferFromExternalAppTemperature component is deprecated. Use 'type = "
             "HeatTransferFromExternalAppTemperature2Phase' instead.");
}
