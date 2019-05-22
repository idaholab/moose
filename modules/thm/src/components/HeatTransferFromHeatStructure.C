#include "HeatTransferFromHeatStructure.h"
#include "HeatStructureBase.h"

registerMooseObject("THMApp", HeatTransferFromHeatStructure);

template <>
InputParameters
validParams<HeatTransferFromHeatStructure>()
{
  InputParameters params = validParams<HeatTransferBase>();
  params.addRequiredParam<std::string>("hs", "The name of the heat structure component");
  params.addRequiredParam<MooseEnum>(
      "hs_side", HeatStructureBase::getSideType(), "The side of the heat structure");
  params.addParam<FunctionName>("Hw", "Convective heat transfer coefficient");
  params.addParam<FunctionName>("Hw_liquid",
                                "Convective one-phase liquid heat transfer coefficient");
  params.addParam<FunctionName>("Hw_vapor", "Convective one-phase vapor heat transfer coefficient");
  return params;
}

HeatTransferFromHeatStructure::HeatTransferFromHeatStructure(const InputParameters & parameters)
  : HeatTransferBase(parameters)
{
}

void
HeatTransferFromHeatStructure::check() const
{
  HeatTransferBase::check();

  if (_model_type == THM::FM_SINGLE_PHASE)
    logError("HeatTransferFromHeatStructure component is deprecated. Use 'type = "
             "HeatTransferFromHeatStructure1Phase' instead.");
  else if (_model_type == THM::FM_TWO_PHASE || _model_type == THM::FM_TWO_PHASE_NCG)
    logError("HeatTransferFromHeatStructure component is deprecated. Use 'type = "
             "HeatTransferFromHeatStructure2Phase' instead.");
}
