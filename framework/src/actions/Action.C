/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "Action.h"
#include "ActionWarehouse.h"
#include "MooseMesh.h"
#include "MooseApp.h"
#include "MooseUtils.h" // remove when getBaseName is removed

template <>
InputParameters
validParams<Action>()
{
  InputParameters params;

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
    _executioner(_app.executioner())
{
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
