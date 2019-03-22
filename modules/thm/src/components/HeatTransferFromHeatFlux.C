#include "HeatTransferFromHeatFlux.h"

registerMooseObject("THMApp", HeatTransferFromHeatFlux);

template <>
InputParameters
validParams<HeatTransferFromHeatFlux>()
{
  InputParameters params = validParams<HeatTransferBase>();
  params.addRequiredParam<FunctionName>("q_wall", "Specified wall heat flux function");
  return params;
}

HeatTransferFromHeatFlux::HeatTransferFromHeatFlux(const InputParameters & parameters)
  : HeatTransferBase(parameters)
{
}

void
HeatTransferFromHeatFlux::check() const
{
  HeatTransferBase::check();

  if (_model_type == THM::FM_SINGLE_PHASE)
    logError("HeatTransferFromHeatFlux component is deprecated. Use 'type = "
             "HeatTransferFromHeatFlux1Phase' instead.");
  else if (_model_type == THM::FM_TWO_PHASE || _model_type == THM::FM_TWO_PHASE_NCG)
    logError("HeatTransferFromHeatFlux component is deprecated. Use 'type = "
             "HeatTransferFromHeatFlux2Phase' instead.");
}
