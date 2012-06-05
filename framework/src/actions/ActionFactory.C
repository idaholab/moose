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

unsigned int ActionFactory::_unique_id = 0;

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
ActionFactory::create(const std::string & action, InputParameters params)
{
  std::pair<ActionFactory::iterator, ActionFactory::iterator> iters;
  BuildInfo *build_info = NULL;
  std::string name = params.have_parameter<std::string>("name") ? params.get<std::string>("name") : "";

  iters = _name_to_build_info.equal_range(action);

  // Find the Action that matches the one we have registered based on unique_id
  unsigned short count = 0;
  for (ActionFactory::iterator it = iters.first; it != iters.second; ++it)
  {
    ++count;
    if (params.have_parameter<unsigned int>("unique_id") && it->second._unique_id == params.get<unsigned int>("unique_id"))
    {
      build_info = &(it->second);
      break;
    }
  }
  // For backwards compatibility - If there is only one Action registered but it doesn't contain a unique_id that
  // matches - then it surely it must still be the correct one
  if (count == 1 && !build_info)
    build_info = &(iters.first->second);

  if (!build_info)
    mooseError(std::string("Unable to find buildable Action from supplied InputParameters Object for ") + name);

  if (params.get<std::string>("action") == "")
    params.set<std::string>("action") = build_info->_action_name;

//  std::cerr << "*** " << params.get<std::string>("name") << " " << params.get<std::string>("action") << " ***\n";

  return (*build_info->_build_pointer)(name, params);
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

  InputParameters params = (iter->second._params_pointer)();

  params.addPrivateParam<unsigned int>("unique_id", iter->second._unique_id);

  // Add a default name which can be overridden by the parser or whatever other future driver
  params.addPrivateParam<std::string>("name", iter->second._action_name);

  return params;
}

std::string
ActionFactory::getActionName(const std::string & action)
{
  // We are returning only the first found instance here
  std::multimap<std::string, BuildInfo>::iterator iter = _name_to_build_info.find(action);

  if (iter != _name_to_build_info.end())
    return iter->second._action_name;
  else
    return "";
}
