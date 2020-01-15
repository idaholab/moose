#include "HeatSourceVolumetric.h"
#include "FlowChannelBase.h"
#include "FlowChannel1Phase.h"
#include "FlowChannel2Phase.h"
#include "FlowModelSinglePhase.h"
#include "FlowModelTwoPhase.h"

registerMooseObject("THMApp", HeatSourceVolumetric);

template <>
InputParameters
validParams<HeatSourceVolumetric>()
{
  InputParameters params = validParams<Component>();
  params.addRequiredParam<std::string>("flow_channel",
                                       "Flow channel name in which to apply heat source");
  params.addRequiredParam<FunctionName>("q", "Volumetric heat source [W/m^3]");
  params.addClassDescription("Volumetric heat source applied on a flow channel");
  return params;
}

HeatSourceVolumetric::HeatSourceVolumetric(const InputParameters & parameters)
  : Component(parameters)
{
}

void
HeatSourceVolumetric::check() const
{
  Component::check();

  checkComponentOfTypeExists<FlowChannelBase>("flow_channel");
}

void
HeatSourceVolumetric::addMooseObjects()
{
  const FlowChannelBase & fch = getComponent<FlowChannelBase>("flow_channel");

  if (dynamic_cast<const FlowChannel1Phase *>(&fch) != nullptr)
  {
    std::string class_name = "OneD3EqnEnergyHeatSource";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
    pars.set<std::vector<SubdomainName>>("block") = fch.getSubdomainNames();
    pars.set<FunctionName>("q") = getParam<FunctionName>("q");
    pars.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    _sim.addKernel(class_name, genName(name(), "rhoE_heat_source"), pars);
  }
  else if (dynamic_cast<const FlowChannel2Phase *>(&fch) != nullptr)
  {
    {
      std::string class_name = "OneD7EqnEnergyHeatSource";
      InputParameters pars = _factory.getValidParams(class_name);
      pars.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID;
      pars.set<std::vector<SubdomainName>>("block") = fch.getSubdomainNames();
      pars.set<FunctionName>("q") = getParam<FunctionName>("q");
      pars.set<MaterialPropertyName>("alpha") = {FlowModelTwoPhase::VOLUME_FRACTION_LIQUID};
      pars.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
      pars.set<std::vector<VariableName>>("beta") = {FlowModelTwoPhase::BETA};
      _sim.addKernel(class_name, genName(name(), "arhoEL_heat_source"), pars);
    }
    {
      std::string class_name = "OneD7EqnEnergyHeatSource";
      InputParameters pars = _factory.getValidParams(class_name);
      pars.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR;
      pars.set<std::vector<SubdomainName>>("block") = fch.getSubdomainNames();
      pars.set<FunctionName>("q") = getParam<FunctionName>("q");
      pars.set<MaterialPropertyName>("alpha") = {FlowModelTwoPhase::VOLUME_FRACTION_VAPOR};
      pars.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
      pars.set<std::vector<VariableName>>("beta") = {FlowModelTwoPhase::BETA};
      _sim.addKernel(class_name, genName(name(), "arhoEV_heat_source"), pars);
    }
  }
}
