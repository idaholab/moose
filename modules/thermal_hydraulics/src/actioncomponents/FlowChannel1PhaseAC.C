//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowChannel1PhaseAC.h"
#include "FlowChannel1PhasePhysics.h"
#include "ActionFactory.h"

// Registered under the classic Component's name ('FlowChannel1Phase') rather than this class's own
// name, so an input file can move a flow channel from [Components] to [ActionComponents] without
// also renaming 'type'. The C++ class itself keeps its 'AC' suffix to stay distinguishable from
// the classic FlowChannel1Phase Component in the source code.
registerMooseActionAliased("ThermalHydraulicsApp",
                           FlowChannel1PhaseAC,
                           "FlowChannel1Phase",
                           "add_mesh_generator");
// FlowChannel1PhaseAC creates and attaches its own FlowChannel1PhasePhysics; see the constructor
registerMooseActionAliased("ThermalHydraulicsApp",
                           FlowChannel1PhaseAC,
                           "FlowChannel1Phase",
                           "init_component_physics");
registerActionComponentAliased("ThermalHydraulicsApp", FlowChannel1PhaseAC, "FlowChannel1Phase");

InputParameters
FlowChannel1PhaseAC::validParams()
{
  InputParameters params = ActionComponent::validParams();
  params += ComponentMeshTransformHelper::validParams();
  params += GravityInterface::validParams();
  params.addClassDescription(
      "A 1D single-phase flow channel, with its mesh generated through the MeshGenerator system.");

  params.addRequiredRangeCheckedParam<Real>("length", "length > 0", "Length of the flow channel");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "n_elems", "n_elems > 0", "Number of elements along the flow channel");

  // These are forwarded as-is to the FlowChannel1PhasePhysics this component creates (see the
  // constructor) - kept here, rather than requiring a separate [Physics] block, so that an input
  // file can move a flow channel from [Components]/FlowChannel1Phase to
  // [ActionComponents]/FlowChannel1PhaseAC with no other changes.
  params.addRequiredParam<UserObjectName>("fp",
                                          "The name of the user object that defines fluid "
                                          "properties");
  params.addRequiredParam<FunctionName>(
      "A", "Area of the flow channel, can be a constant or a function");
  params.addRequiredParam<FunctionName>("initial_p", "Initial pressure [Pa]");
  params.addRequiredParam<FunctionName>("initial_T", "Initial temperature [K]");
  params.addRequiredParam<FunctionName>("initial_vel", "Initial velocity [m/s]");
  params.addRequiredParam<std::vector<Real>>(
      "scaling_factor_1phase",
      "Scaling factors for each single phase variable (rhoA, rhouA, rhoEA)");
  params.addParam<std::vector<std::string>>(
      "closures",
      {},
      "Closures object(s) providing wall friction. This is optional since closure relations can "
      "be supplied directly by Materials as well.");
  params.addParam<FunctionName>("f", "Wall friction factor [-]");
  params.addParam<FunctionName>("D_h", "Hydraulic diameter [m]");
  params.addParam<Real>("roughness", 0.0, "Roughness [m]");
  params.addParam<Real>("PoD", 1, "Pitch-to-diameter ratio for parallel bundle heat transfer [-]");

  // Use THM's classic naming for the flow channel's orientation, instead of
  // ComponentMeshTransformHelper's generic 'direction'. InputParameters::renameParam() redirects
  // ComponentMeshTransformHelper's internal queryParam("direction") calls to the 'orientation'
  // value, so no change is needed in the mixin itself.
  params.renameParam("direction",
                     "orientation",
                     "Direction to orient the flow channel with, assuming it is initially "
                     "oriented along the X-axis (1, 0, 0).");

  return params;
}

FlowChannel1PhaseAC::FlowChannel1PhaseAC(const InputParameters & params)
  : ActionComponent(params),
    ComponentMeshTransformHelper(params),
    GravityInterface(params),
    _length(getParam<Real>("length")),
    _n_elems(getParam<unsigned int>("n_elems")),
    _physics(nullptr)
{
  _dimension = 1;
  // ComponentMeshTransformHelper adds its own required task
  addRequiredTask("add_mesh_generator");
  addRequiredTask("init_component_physics");

  // Build and register the physics defining this flow channel's governing equations, mirroring
  // how AddActionComponentAction itself builds this component - so the user does not need to
  // declare a separate [Physics] block (see class documentation).
  const std::string physics_type = "FlowChannel1PhasePhysics";
  InputParameters physics_params = _action_factory.getValidParams(physics_type);
  physics_params.blockFullpath() = params.blockFullpath();
  physics_params.set<bool>("_built_by_moose") = true;
  physics_params.set<std::string>("registered_identifier") = "(AutoBuilt)";

  physics_params.set<UserObjectName>("fp") = getParam<UserObjectName>("fp");
  physics_params.set<FunctionName>("A") = getParam<FunctionName>("A");
  physics_params.set<FunctionName>("initial_p") = getParam<FunctionName>("initial_p");
  physics_params.set<FunctionName>("initial_T") = getParam<FunctionName>("initial_T");
  physics_params.set<FunctionName>("initial_vel") = getParam<FunctionName>("initial_vel");
  physics_params.set<std::vector<Real>>("scaling_factor_1phase") =
      getParam<std::vector<Real>>("scaling_factor_1phase");
  physics_params.set<RealVectorValue>("gravity_vector") = _gravity_vector;
  physics_params.set<std::vector<std::string>>("closures") =
      getParam<std::vector<std::string>>("closures");
  if (isParamValid("f"))
    physics_params.set<FunctionName>("f") = getParam<FunctionName>("f");
  if (isParamValid("D_h"))
    physics_params.set<FunctionName>("D_h") = getParam<FunctionName>("D_h");
  physics_params.set<Real>("roughness") = getParam<Real>("roughness");
  physics_params.set<Real>("PoD") = getParam<Real>("PoD");

  auto physics_action = _action_factory.create(physics_type, name() + "_physics", physics_params);
  _physics = dynamic_cast<FlowChannel1PhasePhysics *>(physics_action.get());
  if (!_physics)
    mooseError("Internal error: failed to create this component's FlowChannel1PhasePhysics");
  _awh.addActionBlock(physics_action);
}

void
FlowChannel1PhaseAC::addMeshGenerators()
{
  InputParameters params = _factory.getValidParams("GeneratedMeshGenerator");
  params.set<MooseEnum>("dim") = "1";
  params.set<Real>("xmax") = _length;
  params.set<unsigned int>("nx") = _n_elems;
  params.set<std::string>("boundary_name_prefix") = name();
  params.set<SubdomainName>("subdomain_name") = name();
  _app.getMeshGeneratorSystem().addMeshGenerator(
      "GeneratedMeshGenerator", name() + "_base", params);
  _mg_names.push_back(name() + "_base");
  _blocks.push_back(name());

  // Rename the generated boundaries to match THM's <name>:in / <name>:out convention, used by
  // classic THM Components (Component1D) for inlet/outlet/junction boundary references.
  InputParameters rename_params = _factory.getValidParams("RenameBoundaryGenerator");
  rename_params.set<MeshGeneratorName>("input") = _mg_names.back();
  rename_params.set<std::vector<BoundaryName>>("old_boundary") = {name() + "_left",
                                                                  name() + "_right"};
  rename_params.set<std::vector<BoundaryName>>("new_boundary") = {name() + ":in", name() + ":out"};
  _app.getMeshGeneratorSystem().addMeshGenerator(
      "RenameBoundaryGenerator", name() + "_renamed", rename_params);
  _mg_names.push_back(name() + "_renamed");

  _top_mg_name = _mg_names.back();

  ComponentMeshTransformHelper::addMeshGenerators();
}

void
FlowChannel1PhaseAC::addPhysics()
{
  _physics->addComponent(*this);
}

const FlowChannel1PhasePhysics &
FlowChannel1PhaseAC::getPhysics() const
{
  mooseAssert(_physics, "The physics has not been created yet");
  return *_physics;
}
