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

template<>
InputParameters validParams<Action>()
{
  InputParameters params;
  std::vector<std::string> blocks(1);
  blocks[0] = "__all__";

  // Add the "active" parameter to all blocks to support selective child visitation (turn blocks on and off without comments)
  params.addParam<std::vector<std::string> >("active", blocks, "If specified only the blocks named will be visited and made active");
  params.addPrivateParam<std::string>("action");
  params.addPrivateParam<ActionWarehouse *>("awh", NULL);
  return params;
}

Action::Action(const std::string & name, InputParameters params) :
    _name(name),
    _pars(params),
    _app(*getParam<MooseApp *>("_moose_app")),
    _factory(_app.getFactory()),
    _action_factory(_app.getActionFactory()),
    _current_action(getParam<std::string>("action")),
    _awh(*getParam<ActionWarehouse *>("awh")),
    _mesh(_awh.mesh()),
    _displaced_mesh(_awh.displacedMesh()),
    _problem(_awh.problem()),
    _executioner(_awh.executioner())
{
}

std::string
Action::getShortName() const
{
  return _name.substr(_name.find_last_of('/') != std::string::npos ? _name.find_last_of('/') + 1 : 0);
}
