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

#include "Parser.h"

template<>
InputParameters validParams<Action>()
{
  InputParameters params;
  std::vector<std::string> blocks(1);
  blocks[0] = "__all__";

  // Add the "active" parameter to all blocks to support selective child visitation (turn blocks on and off without comments)
  params.addParam<std::vector<std::string> >("active", blocks, "If specified only the blocks named will be visited and made active");
  params.addPrivateParam<std::string>("action");
  params.addPrivateParam<Parser *>("parser_handle");
  return params;
}


Action::Action(const std::string & name, InputParameters params) :
    _name(name),
    _pars(params),
    _action(getParam<std::string>("action")),
    _parser_handle(*getParam<Parser *>("parser_handle")),
    _problem(_parser_handle._problem)
{
}

std::string
Action::getShortName() const
{
  return _name.substr(_name.find_last_of('/') != std::string::npos ? _name.find_last_of('/') + 1 : 0);
}

void
Action::addParamsPtrs(std::vector<InputParameters *> & param_ptrs)
{
  param_ptrs.push_back(&_pars);
}
