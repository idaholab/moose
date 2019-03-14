#include "HeatTransferFromTemperature1Phase.h"
#include "FlowModelSinglePhase.h"
#include "FlowModelTwoPhase.h"
#include "HeatConductionModel.h"
#include "FlowChannel.h"

template <>
InputParameters
validParams<HeatTransferFromTemperature1Phase>()
{
  InputParameters params = validParams<HeatTransfer1PhaseBase>();
  return params;
}

HeatTransferFromTemperature1Phase::HeatTransferFromTemperature1Phase(
    const InputParameters & parameters)
  : HeatTransfer1PhaseBase(parameters)
{
}

void
HeatTransferFromTemperature1Phase::addVariables()
{
  HeatTransfer1PhaseBase::addVariables();

  _sim.addVariable(
      false, FlowModel::TEMPERATURE_WALL, HeatConductionModel::feType(), _block_ids_pipe);
  _sim.addVariable(false, _T_wall_name, HeatConductionModel::feType(), _block_ids_pipe);
}

void
HeatTransferFromTemperature1Phase::addMooseObjects()
{
  HeatTransfer1PhaseBase::addMooseObjects();
}

void
HeatTransferFromTemperature1Phase::addHeatTransferKernels()
{
  {
    const std::string class_name = "OneDEnergyWallHeating";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
    params.set<std::vector<SubdomainName>>("block") = {_pipe_name};
    params.set<std::vector<VariableName>>("T_wall") = {_T_wall_name};
    params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
    params.set<MaterialPropertyName>("Hw") = _Hw_1phase_name;
    params.set<std::vector<VariableName>>("P_hf") = {_P_hf_name};
    params.set<MaterialPropertyName>("T") = FlowModelSinglePhase::TEMPERATURE;
    _sim.addKernel(class_name, genName(name(), "wall_heat_transfer"), params);
  }
}

bool
HeatTransferFromTemperature1Phase::isTemperatureType() const
{
  return true;
}
