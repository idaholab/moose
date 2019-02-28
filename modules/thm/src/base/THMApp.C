#include "THMApp.h"
#include "THMSyntax.h"

#include "ModulesApp.h"
#include "AppFactory.h"
#include "Simulation.h"

#include "SinglePhaseFluidProperties.h"
#include "TwoPhaseFluidProperties.h"
#include "TwoPhaseNCGFluidProperties.h"

std::map<THM::FlowModelID, std::string> THMApp::_flow_model_map;

std::map<THM::FlowModelID, std::map<std::string, std::string>> THMApp::_closures_class_names_map;

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
  params.addCommandLineParam<std::string>(
      "check_jacobian", "--check-jacobian", "To indicate we are checking jacobians");
  params.addCommandLineParam<std::string>(
      "print_component_loops", "--print-component-loops", "Flag to print component loops");
  params.addCommandLineParam<std::string>(
      "count_iterations",
      "--count-iterations",
      "Flag to add postprocessors for linear and nonlinear iterations");
  return params;
}

THMApp::THMApp(InputParameters parameters)
  : MooseApp(parameters), _sim(nullptr), _check_jacobian(false)
{
  THMApp::registerAll(_factory, _action_factory, _syntax);
}

THMApp::~THMApp() { delete _sim; }

void
THMApp::buildSimulation()
{
  _sim = new Simulation(_action_warehouse);
}

void
THMApp::build()
{
  _action_warehouse.executeAllActions();
  _sim->build();
}

void
THMApp::setupOptions()
{
  if (isParamValid("check_jacobian"))
    _check_jacobian = true;

  buildSimulation();
  _pars.set<Simulation *>("_sim") = _sim;

  MooseApp::setupOptions();
  if (Moose::_warnings_are_errors)
    _log.setWarningsAsErrors();

  if (isParamValid("print_component_loops"))
  {
    build();
    _sim->printComponentLoops();
    _ready_to_exit = true;
  }

  if (isParamValid("count_iterations"))
    _sim->addIterationCountPostprocessors();
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

  registerClosuresOption("simple", "Closures1PhaseSimple", THM::FM_SINGLE_PHASE);
  registerClosuresOption("simple", "Closures2PhaseSimple", THM::FM_TWO_PHASE);
  registerClosuresOption("simple", "Closures2PhaseSimple", THM::FM_TWO_PHASE_NCG);

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

void
THMApp::registerApps()
{
  registerApp(THMApp);
}

const std::string &
THMApp::getClosuresClassName(const std::string & closures_option,
                             const THM::FlowModelID & flow_model_id) const
{
  const std::string closures_option_lc = MooseUtils::toLower(closures_option);

  if (_closures_class_names_map.find(flow_model_id) != _closures_class_names_map.end())
  {
    const auto & map_for_flow_model = _closures_class_names_map.at(flow_model_id);
    if (map_for_flow_model.find(closures_option_lc) != map_for_flow_model.end())
      return map_for_flow_model.at(closures_option_lc);
    else
      mooseError("The closures option '" + closures_option_lc + "' is not registered.");
  }
  else
    mooseError("The closures option '" + closures_option_lc + "' is not registered.");
}

void
THMApp::registerClosuresOption(const std::string & closures_option,
                               const std::string & class_name,
                               const THM::FlowModelID & flow_model_id)
{
  const std::string closures_option_lc = MooseUtils::toLower(closures_option);
  _closures_class_names_map[flow_model_id][closures_option_lc] = class_name;
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
