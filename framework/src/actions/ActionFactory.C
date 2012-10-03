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

#include "ActionFactory.h"
#include "Parser.h"

// Static Member initialization
ActionFactory *ActionFactory::_instance = NULL;

// Action Factory Members
ActionFactory *ActionFactory::instance()
{
  if (!_instance)
    _instance = new ActionFactory;

  return _instance;
}

void
ActionFactory::release()
{
  delete _instance;
}

ActionFactory::~ActionFactory()
{
}

Action *
ActionFactory::create(const std::string & action, const std::string & name, InputParameters params)
{
  std::map<std::string, BuildInfo>::iterator it = _name_to_build_info.find(action);

  if (it == _name_to_build_info.end())
    mooseError(std::string("Unable to find buildable Action from supplied InputParameters Object for ") + name);
  else
  {
    BuildInfo & build_info = (*it).second;
    if (params.get<std::string>("action") == "")
      params.set<std::string>("action") = build_info._action_name;
    return (build_info._build_pointer)(name, params);
  }

}

InputParameters
ActionFactory::getValidParams(const std::string & name)
{
  /**
   * If an Action is registered more than once, it'll appear in the _name_to_build_info data
   * structure multiple times.  The actual parameters function remains the same however
   * so we can safely use the first instance
   */
  ActionFactory::iterator iter = _name_to_build_info.find(name);

  if (iter == _name_to_build_info.end())
    mooseError(std::string("A '") + name + "' is not a registered Action\n\n");

  return (iter->second._params_pointer)();
}

std::string
ActionFactory::getActionName(const std::string & action)
{
  // We are returning only the first found instance here
  std::map<std::string, BuildInfo>::iterator iter = _name_to_build_info.find(action);

  if (iter != _name_to_build_info.end())
    return iter->second._action_name;
  else
    return "";
}
