//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Action.h"
#include "ActionWarehouse.h"
#include "MooseApp.h"
#include "MooseTypes.h"
#include "MooseUtils.h" // remove when getBaseName is removed
#include "MooseMesh.h"
#include "FEProblemBase.h"
#include "DisplacedProblem.h"
#include "RelationshipManager.h"

template <>
InputParameters
validParams<Action>()
{
  InputParameters params = emptyInputParameters();

  /**
   * Add the "active" and "inactive" parameters so that all blocks in the input file can selectively
   * create white or black lists of active/inactive sub-blocks.
   */
  params.addParam<std::vector<std::string>>(
      "active",
      std::vector<std::string>({"__all__"}),
      "If specified only the blocks named will be visited and made active");
  params.addParam<std::vector<std::string>>(
      "inactive",
      std::vector<std::string>(),
      "If specified blocks matching these identifiers will be skipped.");

  params.addPrivateParam<std::string>("_moose_docs_type",
                                      "action"); // the type of syntax for documentation system
  params.addPrivateParam<std::string>("_action_name"); // the name passed to ActionFactory::create
  params.addPrivateParam<std::string>("task");
  params.addPrivateParam<std::string>("registered_identifier");
  params.addPrivateParam<std::string>("action_type");
  params.addPrivateParam<ActionWarehouse *>("awh", NULL);

  return params;
}

Action::Action(InputParameters parameters)
  : ConsoleStreamInterface(
        *parameters.getCheckedPointerParam<MooseApp *>("_moose_app", "In Action constructor")),
    PerfGraphInterface(
        parameters.getCheckedPointerParam<MooseApp *>("_moose_app", "In Action constructor")
            ->perfGraph(),
        "Action" +
            (parameters.get<std::string>("action_type") != ""
                 ? std::string("::") + parameters.get<std::string>("action_type")
                 : "") +
            (parameters.get<std::string>("_action_name") != ""
                 ? std::string("::") + parameters.get<std::string>("_action_name")
                 : "") +
            (parameters.isParamValid("task") && parameters.get<std::string>("task") != ""
                 ? std::string("::") + parameters.get<std::string>("task")
                 : "")),
    _pars(parameters),
    _registered_identifier(isParamValid("registered_identifier")
                               ? getParam<std::string>("registered_identifier")
                               : ""),
    _name(getParam<std::string>("_action_name")),
    _action_type(getParam<std::string>("action_type")),
    _app(*getCheckedPointerParam<MooseApp *>("_moose_app", "In Action constructor")),
    _factory(_app.getFactory()),
    _action_factory(_app.getActionFactory()),
    _specific_task_name(_pars.isParamValid("task") ? getParam<std::string>("task") : ""),
    _awh(*getCheckedPointerParam<ActionWarehouse *>("awh")),
    _current_task(_awh.getCurrentTaskName()),
    _mesh(_awh.mesh()),
    _displaced_mesh(_awh.displacedMesh()),
    _problem(_awh.problemBase()),
    _act_timer(registerTimedSection("act", 4))
{
}

void
Action::timedAct()
{
  TIME_SECTION(_act_timer);
  act();
}

void
Action::addRelationshipManager(
    Moose::RelationshipManagerType input_rm_type,
    const InputParameters & moose_object_pars,
    std::string rm_name,
    Moose::RelationshipManagerType rm_type,
    Moose::RelationshipManagerInputParameterCallback rm_input_parameter_func,
    Moose::VarKindType sys_type)
{
  // These need unique names
  static unsigned int unique_object_id = 0;

  auto new_name = moose_object_pars.get<std::string>("_moose_base") + '_' + name() + '_' + rm_name +
                  "_" + Moose::stringify(rm_type) + " " + std::to_string(unique_object_id);

  auto rm_params = _factory.getValidParams(rm_name);
  rm_params.set<Moose::RelationshipManagerType>("rm_type") = rm_type;
  rm_params.set<std::string>("for_whom") = name();
  rm_params.set<Moose::VarKindType>("system_type") = sys_type;

  // Figure out if we shouldn't be adding this one yet
  if (((rm_type & input_rm_type) != input_rm_type)

      || // Or are we adding Geometric but this one needs to be delayed

      (((input_rm_type & Moose::RelationshipManagerType::GEOMETRIC) ==
        Moose::RelationshipManagerType::GEOMETRIC) &&
       ((rm_type & Moose::RelationshipManagerType::GEOMETRIC) ==
        Moose::RelationshipManagerType::GEOMETRIC) &&
       !rm_params.template get<bool>("attach_geometric_early"))

      || // Or is this an Algebraic/Coupling and Geometric one that we already added earlier?

      (((input_rm_type != Moose::RelationshipManagerType::GEOMETRIC) &&
        (rm_type != Moose::RelationshipManagerType::GEOMETRIC) &&
        ((rm_type & Moose::RelationshipManagerType::GEOMETRIC) ==
         Moose::RelationshipManagerType::GEOMETRIC) &&
        rm_params.template get<bool>("attach_geometric_early"))))
    return;

  // If there is a callback for setting the RM parameters let's use it
  if (rm_input_parameter_func)
    rm_input_parameter_func(moose_object_pars, rm_params);

  // If we're doing geometric but we can't build it early - then let's not build it yet
  // (It will get built when we do algebraic)
  if ((input_rm_type & Moose::RelationshipManagerType::GEOMETRIC) ==
          Moose::RelationshipManagerType::GEOMETRIC &&
      !rm_params.get<bool>("attach_geometric_early"))
  {
    // We also need to tell the mesh not to delete remote elements yet
    // Note this will get reset in AddRelationshipManager::act() when attaching Algebraic
    _mesh->getMesh().allow_remote_element_removal(false);

    if (_problem->getDisplacedProblem())
      _problem->getDisplacedProblem()->mesh().getMesh().allow_remote_element_removal(false);

    // Keep looking for more RMs
    return;
  }
  // Ok the above block may have told the mesh not to allow remote element removal during the
  // initial MeshBase::prepare_for_use, which is called after attaching geometric ghosting
  // functors. If we did tell the mesh not to allow remote element removal **and** we're using a
  // DistributedMesh, then we need to tell the mesh to allow remote element removal and ensure
  // that the mesh will delete its remote elements after the EquationSystems init
  else if (input_rm_type != Moose::RelationshipManagerType::GEOMETRIC &&
           (rm_type & Moose::RelationshipManagerType::GEOMETRIC) ==
               Moose::RelationshipManagerType::GEOMETRIC &&
           !rm_params.get<bool>("attach_geometric_early") && _mesh->isDistributedMesh())
  {
    _mesh->needsRemoteElemDeletion(true);
    if (_displaced_mesh)
      _displaced_mesh->needsRemoteElemDeletion(true);
  }

  rm_params.set<MooseMesh *>("mesh") = _mesh.get();

  if (rm_params.areAllRequiredParamsValid())
  {
    auto rm_obj = _factory.create<RelationshipManager>(rm_name, new_name, rm_params);

    // Delete the resources created on behalf of the RM if it ends up not being added to the App.
    if (!_app.addRelationshipManager(rm_obj))
      _factory.releaseSharedObjects(*rm_obj);
    else // we added it
      unique_object_id++;
  }
  else
    mooseError("Missing required parameters for RelationshipManager " + rm_name + " for object " +
               name());
}

void Action::addRelationshipManagers(Moose::RelationshipManagerType) {}

void
Action::addRelationshipManagers(Moose::RelationshipManagerType input_rm_type,
                                const InputParameters & moose_object_pars)
{
  const auto & buildable_types = moose_object_pars.getBuildableRelationshipManagerTypes();

  for (const auto & buildable_type : buildable_types)
  {
    auto & rm_name = std::get<0>(buildable_type);
    auto & rm_type = std::get<1>(buildable_type);
    auto rm_input_parameter_func = std::get<2>(buildable_type);

    // We need to be smart. If an object requested a relationship manager that is both an
    // algebraic and coupling functor, then we need to duplicate that object. This is because we
    // need separate objects for non-linear and auxiliary systems. The non-linear system will use a
    // custom CouplingMatrix based on that specified in the input file for optimal sparsity. The
    // auxiliary system will use a null CouplingMatrix (e.g. all solution vector entries on ghosted
    // elements should be evaluable)

    // check whether we need to limit the systems (and subsequent dof_maps) that this rm can be
    // applied to
    auto evaluable_coupleable =
        Moose::RelationshipManagerType::COUPLING | Moose::RelationshipManagerType::ALGEBRAIC;
    auto is_evaluable_and_coupleable = (rm_type & evaluable_coupleable) == evaluable_coupleable;
    auto is_coupleable = (rm_type & Moose::RelationshipManagerType::COUPLING) ==
                         Moose::RelationshipManagerType::COUPLING;
    auto sys_type = is_coupleable ? Moose::VarKindType::VAR_NONLINEAR : Moose::VarKindType::VAR_ANY;

    addRelationshipManager(
        input_rm_type, moose_object_pars, rm_name, rm_type, rm_input_parameter_func, sys_type);

    if (is_evaluable_and_coupleable)
    {
      auto duplicate_rm_type = Moose::RelationshipManagerType::ALGEBRAIC;
      sys_type = Moose::VarKindType::VAR_AUXILIARY;
      addRelationshipManager(input_rm_type,
                             moose_object_pars,
                             rm_name,
                             duplicate_rm_type,
                             rm_input_parameter_func,
                             sys_type);
    }
  }
}

/// DEPRECATED METHODS
std::string
Action::getShortName() const
{
  mooseDeprecated("getShortName() is deprecated.");
  return MooseUtils::shortName(_name);
}

std::string
Action::getBaseName() const
{
  mooseDeprecated("getBaseName() is deprecated.");
  return MooseUtils::baseName(_name);
}
