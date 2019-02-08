#include "THMApp.h"
#include "THMSyntax.h"

#include "ModulesApp.h"
#include "AppFactory.h"

#include "SinglePhaseFluidProperties.h"
#include "TwoPhaseFluidProperties.h"
#include "TwoPhaseNCGFluidProperties.h"

std::map<THM::FlowModelID, std::string> THMApp::_flow_model_map;

std::set<std::string> THMApp::_closure_types;
std::string THMApp::_default_closure_type;
std::map<std::string, std::string> THMApp::_whtc_3eqn_name_map;
std::map<std::string, std::string> THMApp::_wfc_3eqn_name_map;
std::map<std::string, std::string> THMApp::_whtc_7eqn_name_map;
std::map<std::string, std::string> THMApp::_wfc_7eqn_name_map;
std::map<std::string, std::string> THMApp::_iht_name_map;
std::map<std::string, std::string> THMApp::_ifc_name_map;
std::map<std::string, std::string> THMApp::_sia_name_map;
std::map<std::string, std::string> THMApp::_frm_name_map;
std::set<std::string> THMApp::_chf_table_types;
std::string THMApp::_default_chf_table_type;
std::map<std::string, std::string> THMApp::_chf_name_map;

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
FlowModelID FM_TWO_PHASE = registerFlowModelID();
FlowModelID FM_TWO_PHASE_NCG = registerFlowModelID();

} // namespace THM

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
  registerFlowModel(THM::FM_TWO_PHASE, FlowModelTwoPhase);
  registerFlowModel(THM::FM_TWO_PHASE_NCG, FlowModelTwoPhaseNCG);
}

const THM::FlowModelID &
THMApp::getFlowModelID(const FluidProperties & fp)
{
  if (dynamic_cast<const SinglePhaseFluidProperties *>(&fp) != nullptr)
    return THM::FM_SINGLE_PHASE;
  else if (dynamic_cast<const TwoPhaseNCGFluidProperties *>(&fp) != nullptr)
    return THM::FM_TWO_PHASE_NCG;
  else if (dynamic_cast<const TwoPhaseFluidProperties *>(&fp) != nullptr)
    return THM::FM_TWO_PHASE;
  else
    raiseFlowModelError(fp,
                        "one of the following types: 'SinglePhaseFluidProperties', "
                        "'TwoPhaseFluidProperties', or 'TwoPhaseNCGFluidProperties'");
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

const std::string &
THMApp::getClosureMapEntry(const std::map<std::string, std::string> & closure_map,
                           const std::string & closure_name,
                           const std::string & description) const
{
  const std::string closure_name_lower_case = MooseUtils::toLower(closure_name);

  if (closure_map.find(closure_name_lower_case) == closure_map.end())
    mooseError("The closure '" + closure_name_lower_case + "' has no registered class for '" +
               description + "'");
  return closure_map.at(closure_name_lower_case);
}

void
THMApp::registerApps()
{
  registerApp(THMApp);
}

void
THMApp::registerClosureType(const std::string & closure_type, bool is_default)
{
  std::string closure_type_lc = MooseUtils::toLower(closure_type);
  _closure_types.insert(closure_type_lc);
  if (is_default)
    _default_closure_type = closure_type_lc;
}

void
THMApp::registerCriticalHeatFluxTableType(const std::string & chf_table_type, bool is_default)
{
  std::string chf_table_type_lc = MooseUtils::toLower(chf_table_type);
  _chf_table_types.insert(chf_table_type_lc);
  if (is_default)
    _default_chf_table_type = chf_table_type_lc;
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

namespace THM
{

// This is chosen to be an arbitrarily large number so that it can be safely
// assumed that it is unique; other boundary ID's are counted up from zero.
boundary_id_type bnd_nodeset_id = std::numeric_limits<boundary_id_type>::max() - 1;
}
