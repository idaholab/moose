//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalHydraulicsApp.h"
#include "THMSyntax.h"
#include "HeatConductionApp.h"
#include "FluidPropertiesApp.h"
#include "NavierStokesApp.h"
#include "RayTracingApp.h"
#include "RdgApp.h"
#include "SolidPropertiesApp.h"
#include "MiscApp.h"
#include "THMProblem.h"

#include "AppFactory.h"

#include "SinglePhaseFluidProperties.h"
#include "TwoPhaseFluidProperties.h"
#include "TwoPhaseNCGFluidProperties.h"

std::map<THM::FlowModelID, std::string> ThermalHydraulicsApp::_flow_model_map;

std::map<THM::FlowModelID, std::map<std::string, std::string>>
    ThermalHydraulicsApp::_closures_class_names_map;

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

InputParameters
ThermalHydraulicsApp::validParams()
{
  InputParameters params = MooseApp::validParams();
  params.set<bool>("use_legacy_output_syntax") = false;
  params.set<bool>("use_legacy_material_output") = false;
  return params;
}

registerKnownLabel("ThermalHydraulicsApp");

ThermalHydraulicsApp::ThermalHydraulicsApp(InputParameters parameters)
  : MooseApp(parameters),
    _thm_mesh(nullptr),
    _component_warehouse(getFactory()),
    _set_thm_mesh(false),
    _thm_problem(nullptr),
    _set_thm_problem_pointer(false)
{
  ThermalHydraulicsApp::registerAll(_factory, _action_factory, _syntax);

  if (getParam<bool>("error"))
    log().setWarningsAsErrors();
}

ThermalHydraulicsApp::~ThermalHydraulicsApp() {}

void
ThermalHydraulicsApp::registerObjects(Factory & /*factory*/)
{
  mooseError("registerObjects() is deprecated and not supported in THM");
}

void
ThermalHydraulicsApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
  mooseError("associateSyntax() is deprecated and not supported in THM");
}

void
ThermalHydraulicsApp::registerExecFlags(Factory & /*factory*/)
{
  mooseError("registerExecFlags() is deprecated and not supported in THM");
}

void
ThermalHydraulicsApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  Registry::registerObjectsTo(f, {"ThermalHydraulicsApp"});
  Registry::registerActionsTo(af, {"ThermalHydraulicsApp"});

  HeatConductionApp::registerAll(f, af, s);
  FluidPropertiesApp::registerAll(f, af, s);
  NavierStokesApp::registerAll(f, af, s);
  RayTracingApp::registerAll(f, af, s);
  RdgApp::registerAll(f, af, s);
  SolidPropertiesApp::registerAll(f, af, s);
  MiscApp::registerAll(f, af, s);

  THM::associateSyntax(s);
  THM::registerActions(s);

  registerClosuresOption("simple", "Closures1PhaseSimple", THM::FM_SINGLE_PHASE);
  registerClosuresOption("none", "Closures1PhaseNone", THM::FM_SINGLE_PHASE);

  // flow models
  registerFlowModel(THM::FM_SINGLE_PHASE, FlowModelSinglePhase);
}

const std::string &
ThermalHydraulicsApp::getFlowModelClassName(const THM::FlowModelID & flow_model_id)
{
  const auto it = _flow_model_map.find(flow_model_id);
  if (it == _flow_model_map.end())
    mooseError("Flow model with ID '" + Moose::stringify(flow_model_id) +
               "' is not associated with any flow models class. Register your flow model class to "
               "a flow model ID by calling registerFlowModel().");
  return it->second;
}

void
ThermalHydraulicsApp::augmentSparsity(const dof_id_type & elem_id1, const dof_id_type & elem_id2)
{
  // augment the first element's sparsity
  auto it = _sparsity_augmentation_map.find(elem_id1);
  if (it == _sparsity_augmentation_map.end())
    it = _sparsity_augmentation_map.insert(_sparsity_augmentation_map.begin(),
                                           {elem_id1, std::vector<dof_id_type>()});
  it->second.push_back(elem_id2);

  // augment the second element's sparsity
  it = _sparsity_augmentation_map.find(elem_id2);
  if (it == _sparsity_augmentation_map.end())
    it = _sparsity_augmentation_map.insert(_sparsity_augmentation_map.begin(),
                                           {elem_id2, std::vector<dof_id_type>()});
  it->second.push_back(elem_id1);
}

void
ThermalHydraulicsApp::registerApps()
{
  registerApp(ThermalHydraulicsApp);
}

const std::string &
ThermalHydraulicsApp::getClosuresClassName(const std::string & closures_option,
                                           const THM::FlowModelID & flow_model_id)
{
  const std::string closures_option_lc = MooseUtils::toLower(closures_option);

  if (_closures_class_names_map.find(flow_model_id) != _closures_class_names_map.end())
  {
    const auto & map_for_flow_model = _closures_class_names_map.at(flow_model_id);
    if (map_for_flow_model.find(closures_option_lc) != map_for_flow_model.end())
    {
      const std::string & closures_class = map_for_flow_model.at(closures_option_lc);
      mooseDeprecated("The closures system now uses objects created in the input file instead of "
                      "enumerated options.\n",
                      "To remove this warning, add the following block to your input file "
                      "(replacing 'my_closures' as you choose):\n",
                      "  [Closures]\n    [my_closures]\n      type = ",
                      closures_class,
                      "\n    []\n  []\n",
                      "Then, set the 'closures' parameter in your flow channel to this name:\n",
                      "  closures = my_closures");
      return closures_class;
    }
    else
      mooseError("The closures option '" + closures_option_lc + "' is not registered.");
  }
  else
    mooseError("The closures option '" + closures_option_lc + "' is not registered.");
}

void
ThermalHydraulicsApp::registerClosuresOption(const std::string & closures_option,
                                             const std::string & class_name,
                                             const THM::FlowModelID & flow_model_id)
{
  const std::string closures_option_lc = MooseUtils::toLower(closures_option);
  _closures_class_names_map[flow_model_id][closures_option_lc] = class_name;
}

std::shared_ptr<THMMesh> &
ThermalHydraulicsApp::getTHMMesh()
{
  if (_set_thm_mesh)
    return _thm_mesh;
  else
    mooseError("The THM mesh has not been set.");
}

void
ThermalHydraulicsApp::setTHMMesh(std::shared_ptr<THMMesh> & thm_mesh)
{
  if (_set_thm_mesh)
    mooseError("The THM mesh has already been set.");
  else
  {
    _thm_mesh = thm_mesh;
    _set_thm_mesh = true;
  }
}

THMProblem &
ThermalHydraulicsApp::getTHMProblem() const
{
  if (_set_thm_problem_pointer)
    return *_thm_problem;
  else
    mooseError("The THM problem has not been set.");
}

void
ThermalHydraulicsApp::setTHMProblem(FEProblemBase & problem)
{
  auto thm_problem = dynamic_cast<THMProblem *>(&problem);
  if (thm_problem)
  {
    _thm_problem = thm_problem;
    _set_thm_problem_pointer = true;
  }
  else
    mooseError("The problem is not of type THMProblem.");
}

void
ThermalHydraulicsApp::addComponent(const std::string & type,
                                   const std::string & name,
                                   InputParameters & params)
{
  if (hasComponent(name))
    mooseError("Component with name '", name, "' already exists");
  else
    _component_warehouse.addComponent(type, name, params);
}

//
// Dynamic Library Entry Points - DO NOT MODIFY
//
extern "C" void
ThermalHydraulicsApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ThermalHydraulicsApp::registerAll(f, af, s);
}

extern "C" void
ThermalHydraulicsApp__registerApps()
{
  ThermalHydraulicsApp::registerApps();
}
