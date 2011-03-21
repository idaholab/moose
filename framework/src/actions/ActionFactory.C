#include "ActionFactory.h"
#include "Parser.h"

ActionFactory *ActionFactory::instance()
{
  static ActionFactory *instance;
  if (!instance)
    instance = new ActionFactory;

  return instance;
} 

ActionFactory:: ~ActionFactory()
{
  for (std::vector<Action *>::iterator i=_active_parser_blocks.begin(); i!=_active_parser_blocks.end(); ++i)
    delete *i;
}

Action *
ActionFactory::create(const std::string & name, InputParameters params)
{
  Action * action_block;
  std::string generic_identifier = ActionFactory::instance()->isRegistered(name);

  params.set<std::string>("action") = _name_to_action_map[generic_identifier];
  
  action_block = (*_name_to_build_pointer[generic_identifier])(name, params);
  _active_parser_blocks.push_back(action_block);

  return action_block;
}

InputParameters
ActionFactory::getValidParams(const std::string & name)
{
  bool is_parent;
  std::string generic_identifier = ActionFactory::instance()->isRegistered(name, &is_parent);

  // If the is_parent variable is set that means that this block was not registered and we will
  // just return an empty Parameters object and have the parser deal with it
  if (is_parent)
    return InputParameters();
  
  if( _name_to_params_pointer.find(generic_identifier) == _name_to_params_pointer.end() )
    mooseError(std::string("A '") + name + "' is not a registered Action\n\n");

  return _name_to_params_pointer[generic_identifier]();
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

  std::map<std::string, paramsActionPtr>::reverse_iterator i;
  std::vector<std::string> real_elements, reg_elements;
  std::string return_value;

  Parser::tokenize(real_id, real_elements);

  *is_parent = false;
  for (i=_name_to_params_pointer.rbegin(); i!=_name_to_params_pointer.rend(); ++i)
  {
    std::string reg_id = i->first;
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
    InputParameters params = getValidParams(it->second);
    // FIXME: HACK
    params.set<Parser *>("parser_handle") = p_ptr;

    if (params.areAllRequiredParamsValid())
    {
      Moose::action_warehouse.addActionBlock(create(it->second, params));
      ret_value = true;
    }
  }

  return ret_value;
}
