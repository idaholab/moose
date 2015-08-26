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

template<>
InputParameters validParams<Action>()
{
  InputParameters params;
  std::vector<std::string> blocks(1);
  blocks[0] = "__all__";
  // Add the "active" parameter to all blocks to support selective child visitation (turn blocks on and off without comments)
  params.addParam<std::vector<std::string> >("active", blocks, "If specified only the blocks named will be visited and made active");

  params.addPrivateParam<std::string>("task");
  params.addPrivateParam<std::string>("registered_identifier");
  params.addPrivateParam<std::string>("action_type");
  params.addPrivateParam<ActionWarehouse *>("awh", NULL);

  return params;
}

Action::Action(InputParameters parameters) :
    ConsoleStreamInterface(*parameters.getCheckedPointerParam<MooseApp *>("_moose_app", "In Action constructor")),
    _pars(parameters),
    _registered_identifier(isParamValid("registered_identifier") ? getParam<std::string>("registered_identifier") : ""),
    _full_syntax(getParam<std::string>("name")),
    _base_syntax(MooseUtils::baseName(_full_syntax)),
    // If the action was created via the Parser is will have a "registered_identifier" set that gives the input file syntax
    // that generated the action. In this case, the name will contain this syntax (e.g., Kernels/object_name). The true object
    // name is simply "object_name" in this case, thus the name is shortened. If this identifier is empty or set to "(AutoBuilt)"
    // then just leave the name as in the "meta action" case.
    _name( (_registered_identifier.empty() || _registered_identifier == "(AutoBuilt)")  ?
           _full_syntax : MooseUtils::shortName(_full_syntax)),
    _action_type(getParam<std::string>("action_type")),
    _app(*parameters.getCheckedPointerParam<MooseApp *>("_moose_app", "In Action constructor")),
    _factory(_app.getFactory()),
    _action_factory(_app.getActionFactory()),
    _specific_task_name(_pars.isParamValid("task") ? getParam<std::string>("task") : ""),
    _awh(*parameters.getCheckedPointerParam<ActionWarehouse *>("awh")),
    _current_task(_awh.getCurrentTaskName()),
    _mesh(_awh.mesh()),
    _displaced_mesh(_awh.displacedMesh()),
    _problem(_awh.problem()),
    _executioner(_app.executioner())
{
}


/// DEPRECATED METHODS
const std::string &
Action::getShortName() const
{
  mooseDeprecated("getShortName() is deprecated, please use name() method.");
  return _name;
}

std::string
Action::getBaseName() const
{
  mooseDeprecated("getBaseName() is deprecated, please Use getInputSyntaxBase() method.");
  return MooseUtils::baseName(_full_syntax);
}
