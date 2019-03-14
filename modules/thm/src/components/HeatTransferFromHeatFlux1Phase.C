#include "HeatTransferFromHeatFlux1Phase.h"
#include "FlowModelSinglePhase.h"
#include "FlowModelTwoPhase.h"
#include "FlowChannel.h"

registerMooseObject("THMApp", HeatTransferFromHeatFlux1Phase);

template <>
InputParameters
validParams<HeatTransferFromHeatFlux1Phase>()
{
  InputParameters params = validParams<HeatTransfer1PhaseBase>();
  params.addRequiredParam<FunctionName>("q_wall", "Specified wall heat flux function");
  params.addClassDescription(
      "Heat transfer specified by heat flux going into 1-phase flow channel.");
  return params;
}

HeatTransferFromHeatFlux1Phase::HeatTransferFromHeatFlux1Phase(const InputParameters & parameters)
  : HeatTransfer1PhaseBase(parameters), _q_wall_fn_name(getParam<FunctionName>("q_wall"))
{
}

void
HeatTransferFromHeatFlux1Phase::addMooseObjects()
{
  HeatTransfer1PhaseBase::addMooseObjects();

  {
    const std::string class_name = "GenericFunctionMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = {_pipe_name};
    params.set<std::vector<std::string>>("prop_names") = {_q_wall_name};
    params.set<std::vector<FunctionName>>("prop_values") = {_q_wall_fn_name};
    _sim.addMaterial(class_name, genName(name(), "q_wall_material"), params);
  }

  // wall heat transfer kernel
  {
    const std::string class_name = "OneDEnergyWallHeatFlux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
    params.set<std::vector<SubdomainName>>("block") = {_pipe_name};
    params.set<MaterialPropertyName>("q_wall") = _q_wall_name;
    params.set<std::vector<VariableName>>("P_hf") = {_P_hf_name};
    _sim.addKernel(class_name, genName(name(), "wall_heat"), params);
  }
}

bool
HeatTransferFromHeatFlux1Phase::isTemperatureType() const
{
  return false;
}
