#include "THMApp.h"
#include "THMSyntax.h"

#include "ModulesApp.h"
#include "AppFactory.h"

#include "SinglePhaseFluidProperties.h"

std::map<THM::FlowModelID, std::string> THMApp::_flow_model_map;

namespace THM
{

FlowModelID
registerFlowModelID()
{
  static FlowModelID flow_model_id = 0;
  flow_model_id++;
  return flow_model_id;
}

FlowModelID FM_INVALID = registerFlowModelID();
FlowModelID FM_SINGLE_PHASE = registerFlowModelID();
}

template <>
InputParameters
validParams<THMApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

THMApp::THMApp(InputParameters parameters) : MooseApp(parameters)
{
  THMApp::registerAll(_factory, _action_factory, _syntax);
}

void
THMApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  Registry::registerObjectsTo(f, {"THMApp"});
  Registry::registerActionsTo(af, {"THMApp"});

  s.replaceActionSyntax(
      "AddFluidPropertiesAction", "FluidProperties/*", "add_fluid_properties", __FILE__, __LINE__);

  ModulesApp::registerAll(f, af, s);

  THM::associateSyntax(s);
  THM::registerActions(s);

  // flow models
  registerFlowModel(THM::FM_SINGLE_PHASE, FlowModelSinglePhase);
}

const THM::FlowModelID &
THMApp::getFlowModelID(const FluidProperties & fp)
{
  if (dynamic_cast<const SinglePhaseFluidProperties *>(&fp) != nullptr)
    return THM::FM_SINGLE_PHASE;
  else
    raiseFlowModelError(fp, "'SinglePhaseFluidProperties'");
}

const std::string &
THMApp::getFlowModelClassName(const THM::FlowModelID & flow_model_id)
{
  const auto it = _flow_model_map.find(flow_model_id);
  if (it == _flow_model_map.end())
    mooseError("Flow model with ID '" + Moose::stringify(flow_model_id) +
               "' is not associated with any flow models class. Register your flow model class to "
               "a flow model ID by calling registerFlowModel().");
  return it->second;
}

void
THMApp::raiseFlowModelError(const FluidProperties & fp, const std::string & mbdf)
{
  mooseError(
      "The specified fluid properties object, '", fp.name(), "', must be derived from ", mbdf, ".");
}

void
THMApp::registerApps()
{
  registerApp(THMApp);
}

//
// Dynamic Library Entry Points - DO NOT MODIFY
//
extern "C" void
THMApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  THMApp::registerAll(f, af, s);
}

extern "C" void
THMApp__registerApps()
{
  THMApp::registerApps();
}
