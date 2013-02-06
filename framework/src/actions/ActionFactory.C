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

unsigned int ActionFactory::_unique_id = 0;

ActionFactory::ActionFactory(MooseApp & app):
    _app(app)
{
}

ActionFactory::~ActionFactory()
{
}

Action *
ActionFactory::create(const std::string & action, const std::string & name, InputParameters params)
{
  params.set<MooseApp *>("_moose_app") = &_app;
  std::pair<ActionFactory::iterator, ActionFactory::iterator> iters;
  BuildInfo *build_info = NULL;

  // Check to make sure that all required parameters are supplied
  params.checkParams(name);

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
  params.set<MooseApp *>("_moose_app") = &_app;

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
