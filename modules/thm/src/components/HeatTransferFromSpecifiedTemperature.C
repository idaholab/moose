#include "HeatTransferFromSpecifiedTemperature.h"

registerMooseObject("THMApp", HeatTransferFromSpecifiedTemperature);

template <>
InputParameters
validParams<HeatTransferFromSpecifiedTemperature>()
{
  InputParameters params = validParams<HeatTransferBase>();
  params.addRequiredParam<FunctionName>("T_wall", "Specified wall temperature function");
  return params;
}

HeatTransferFromSpecifiedTemperature::HeatTransferFromSpecifiedTemperature(
    const InputParameters & parameters)
  : HeatTransferBase(parameters)
{
}

void
HeatTransferFromSpecifiedTemperature::check() const
{
  HeatTransferBase::check();

  if (_model_type == THM::FM_SINGLE_PHASE)
    logError("HeatTransferFromSpecifiedTemperature component is deprecated. Use 'type = "
             "HeatTransferFromSpecifiedTemperature1Phase' instead.");
  else if (_model_type == THM::FM_TWO_PHASE || _model_type == THM::FM_TWO_PHASE_NCG)
    logError("HeatTransferFromSpecifiedTemperature component is deprecated. Use 'type = "
             "HeatTransferFromSpecifiedTemperature2Phase' instead.");
}
