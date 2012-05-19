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

#include "GlobalParamsAction.h"

template<>
InputParameters validParams<GlobalParamsAction>()
{
  InputParameters params = validParams<Action>();
  std::vector<std::string> blocks(1);
  blocks[0] = "";

  /* GlobalParams should not have children or other standard public Action attributes */
  params.addPrivateParam<std::vector<std::string> >("active", blocks);
  params.addPrivateParam<Parser *>("parser_handle");
  params.addParam<std::string>("add_parameters","add_values","Add your own parameters to this block!");
  return params;
}

GlobalParamsAction::GlobalParamsAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
GlobalParamsAction::act()
{
}
