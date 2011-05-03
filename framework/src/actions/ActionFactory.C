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
ActionFactory::create(const std::string & name, InputParameters params)
{
  std::pair<ActionFactory::iterator, ActionFactory::iterator> iters;
  BuildInfo *build_info = NULL;
  
  std::string generic_identifier = ActionFactory::instance()->isRegistered(name);
  iters = _name_to_build_info.equal_range(generic_identifier);

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

  // action_blocks.push_back((*it->second._build_pointer)(name, p));
  params.set<std::string>("action") = build_info->_action_name;
  return (*build_info->_build_pointer)(name, params);
}
  
  
//   Action * action_block;
//   std::string generic_identifier = ActionFactory::instance()->isRegistered(name);

//   params.set<std::string>("action") = _name_to_action_map[generic_identifier];
 
//   action_block = (*_name_to_build_pointer[generic_identifier])(name, params);
//   _active_parser_blocks.push_back(action_block);

//   return action_block;


//   std::multimap<std::string, BuildInfo>::iterator it;
//   std::pair<std::multimap<std::string, BuildInfo>::iterator, std::multimap<std::string, BuildInfo>::iterator> iters;
  
//   std::vector<Action *> action_blocks;
//   std::string generic_identifier = ActionFactory::instance()->isRegistered(name);

//   iters = _name_to_build_info.equal_range(generic_identifier);
  
//   for (it = iters.first; it != iters.second; ++it)
//   {
//     InputParameters p = params;
//     p.set<std::string>("action") = it->second._action_name;
//     action_blocks.push_back((*it->second._build_pointer)(name, p));
//   }

  
//   //params.set<std::string>("action") = _name_to_action_map[generic_identifier];
  
// //  action_block = (*_name_to_build_pointer[generic_identifier])(name, params);
// //  _active_parser_blocks.push_back(action_block);

//   return action_blocks;
// }


Action *
ActionFactory::createNonParsed(const std::string & name, InputParameters params)
{
  std::multimap<std::string, std::string>::iterator it;
  std::pair<std::multimap<std::string, std::string>::iterator, std::multimap<std::string, std::string>::iterator> iters;

  // First get the names of all the Actions for the given action_name so we can index into the other Factory maps
  iters = _action_to_name_map.equal_range(name);

  for (it = iters.first; it != iters.second; ++it)
  {
    InputParameters p = params;
    if (p.areAllRequiredParamsValid())
      return create(it->second, p);
  }

  return NULL;
}


std::vector<InputParameters>
ActionFactory::getAllValidParams(const std::string & name)
{
  std::pair<ActionFactory::iterator, ActionFactory::iterator> iters;
  std::vector<InputParameters> all_params;
  bool is_parent;
  
  std::string generic_identifier = ActionFactory::instance()->isRegistered(name, &is_parent);
  
  // If the is_parent variable is set, that means that this block was not explicitly registered.
  // We and we will return an empty vector
  if (!is_parent)
  {
    iters = _name_to_build_info.equal_range(generic_identifier);
  
    if (iters.first == iters.second)
      mooseError(std::string("A '") + name + "' is not a registered Action\n\n");

    for (ActionFactory::iterator it = iters.first; it != iters.second; ++it)
    {
      InputParameters params = (*it->second._params_pointer)();

      // Inject the "built_by_action" param and the "unique_id" param
      params.addPrivateParam<std::string>("built_by_action", it->second._action_name);
      params.addPrivateParam<unsigned int>("unique_id", it->second._unique_id);
      
      all_params.push_back(params);
    }
  }
  
  return all_params;
}

InputParameters
ActionFactory::getValidParams(const std::string & name)
{
  std::vector<InputParameters> all = getAllValidParams(name);
  if (all.size() != 1)
    mooseWarning(name + " is double registered in the ActionFactory but you are only retreiving parameters for one object.\nPlease call ""getAllValidParams()"" instead.");

  return all[0];
}


std::string
ActionFactory::isRegistered(const std::string & real_id, bool * is_parent)
{
  /**
   * This implementation assumes that wildcards can occur in the place of an entire token but not as part
   * of a token (i.e.  'Variables/ * /InitialConditions' is valid but not 'Variables/Partial* /InitialConditions'.
   * Since maps are ordered, a reverse traversal through the registered list will always select a more
   * specific match before a wildcard match ('*' == char(42))
   */
  bool local_is_parent;
  if (is_parent == NULL)
    is_parent = &local_is_parent;  // Just so we don't have to keep checking below when we want to set the value

  std::multimap<std::string, BuildInfo>::reverse_iterator it;
  std::vector<std::string> real_elements, reg_elements;
  std::string return_value;

  Parser::tokenize(real_id, real_elements);

  *is_parent = false;
  for (it=_name_to_build_info.rbegin(); it != _name_to_build_info.rend(); ++it)
  {
    std::string reg_id = it->first;
    if (reg_id == real_id)
    {
      *is_parent = false;
      return reg_id;
    }

    reg_elements.clear();
    Parser::tokenize(reg_id, reg_elements);
    if (real_elements.size() <= reg_elements.size())
    {
      bool keep_going = true;
      for (unsigned int j=0; keep_going && j<real_elements.size(); ++j)
      {
        if (real_elements[j] != reg_elements[j] && reg_elements[j] != std::string("*"))
          keep_going = false;
      }
      if (keep_going)
      { 
        if (real_elements.size() < reg_elements.size() && !*is_parent)
        {
          // We found a parent, the longest parent in fact but we need to keep
          // looking to make sure that the real thing isn't registered
          *is_parent = true;
          return_value = reg_id;
        } 
        else if (real_elements.size() == reg_elements.size())
        {
          *is_parent = false;
          return reg_id;
        }
      }
    }
  }

  if (*is_parent)
    return return_value;
  else
    return std::string("");
}

bool
ActionFactory::buildAllBuildableActions(const std::string & action, Parser * p_ptr)
{
  bool ret_value = false;
  std::multimap<std::string, std::string>::iterator it;
  std::pair<std::multimap<std::string, std::string>::iterator, std::multimap<std::string, std::string>::iterator> iters;
  
  // First get the names of all the Actions for the given action_name so we can index into the other Factory maps
  iters = _action_to_name_map.equal_range(action);

  for (it = iters.first; it != iters.second; ++it)
  {
    std::vector<InputParameters> all_params = getAllValidParams(it->second);

    for (std::vector<InputParameters>::iterator jt = all_params.begin(); jt != all_params.end(); ++jt)
    {
      
      // FIXME: HACK
      jt->set<Parser *>("parser_handle") = p_ptr;

      if (jt->areAllRequiredParamsValid())
      {
        Moose::action_warehouse.addActionBlock(create(it->second, *jt));
        ret_value = true;
      }
    }
    
  }

  return ret_value;
}
