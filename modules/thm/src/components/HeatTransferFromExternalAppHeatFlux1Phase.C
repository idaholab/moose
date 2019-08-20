#include "HeatTransferFromExternalAppHeatFlux1Phase.h"
#include "FlowModelSinglePhase.h"
#include "FlowModelTwoPhase.h"

registerMooseObject("THMApp", HeatTransferFromExternalAppHeatFlux1Phase);

template <>
InputParameters
validParams<HeatTransferFromExternalAppHeatFlux1Phase>()
{
  InputParameters params = validParams<HeatTransfer1PhaseBase>();
  params.addClassDescription("Heat transfer specified by heat flux provided by an external "
                             "application going into 1-phase flow channel.");
  return params;
}

HeatTransferFromExternalAppHeatFlux1Phase::HeatTransferFromExternalAppHeatFlux1Phase(
    const InputParameters & parameters)
  : HeatTransfer1PhaseBase(parameters)
{
}

void
HeatTransferFromExternalAppHeatFlux1Phase::addVariables()
{
  HeatTransfer1PhaseBase::addVariables();

  _sim.addVariable(false, _q_wall_name, FEType(CONSTANT, MONOMIAL), _flow_channel_subdomains);
  _sim.addConstantIC(_q_wall_name, 0, _flow_channel_name);
}

void
HeatTransferFromExternalAppHeatFlux1Phase::addMooseObjects()
{
  HeatTransfer1PhaseBase::addMooseObjects();

  {
    const std::string class_name = "CoupledVariableValueMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = _flow_channel_subdomains;
    params.set<MaterialPropertyName>("prop_name") = _q_wall_name;
    params.set<std::vector<VariableName>>("coupled_variable") = {_q_wall_name};
    _sim.addMaterial(class_name, genName(name(), "q_wall_material"), params);
  }

  // wall heat transfer kernel
  {
    const std::string class_name = "OneDEnergyWallHeatFlux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel_subdomains;
    params.set<MaterialPropertyName>("q_wall") = _q_wall_name;
    params.set<std::vector<VariableName>>("P_hf") = {_P_hf_name};
    _sim.addKernel(class_name, genName(name(), "wall_heat"), params);
  }
}

bool
HeatTransferFromExternalAppHeatFlux1Phase::isTemperatureType() const
{
  return false;
}
