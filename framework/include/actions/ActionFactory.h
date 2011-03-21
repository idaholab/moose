#ifndef ACTIONFACTORY_H_
#define ACTIONFACTORY_H_

#include <vector>
#include <map>
#include <set>

#include "Moose.h"
#include "Action.h"
#include "InputParameters.h"
#include "ActionWarehouse.h"

/**
 * Macros
 */
#define stringifyName(name) #name
#define registerActionName(name, is_required)   Moose::action_warehouse.registerName(name, is_required)
#define registerAction(tplt, name, action)      ActionFactory::instance()->reg<tplt>(name, action)
#define registerNonParsedAction(tplt, action)   ActionFactory::instance()->regNonParsed<tplt>(action)

// TODO: This will change when action_warehouse is moved inside of some system
#define addActionNameDependency(action, depends_on)       Moose::action_warehouse.addDependency(action, depends_on)

/**
 * Typedef for function to build objects
 */
typedef Action * (*buildActionPtr)(const std::string & name, InputParameters parameters);

/**
 * Typedef for validParams
 */
typedef InputParameters (*paramsActionPtr)();

/**
 * Typedef for registered Action iterator
 */
typedef std::map<std::string, std::string>::iterator registeredActionIterator;

/**
 * Build an object of type T
 */
template<class T>
Action * buildAction(const std::string & name, InputParameters parameters)
{
  return new T(name, parameters);
}


/**
 * Generic factory class for build all sorts of objects
 */
class ActionFactory
{
public:
  static ActionFactory *instance();

  virtual ~ActionFactory();

  template<typename T>
  void reg(const std::string & name, const std::string & action_name)
  {
    if (_name_to_build_pointer.find(name) == _name_to_build_pointer.end())
    {
      _name_to_build_pointer[name] = &buildAction<T>;
      _name_to_params_pointer[name] = &validParams<T>;
      _name_to_action_map[name] = action_name;
      _action_to_name_map.insert(std::make_pair(action_name, name));
    }
    else
      mooseError("Action '" + name + "' already registered.");
  }

  template<typename T>
  void regNonParsed(const std::string & action_name)
  {
    std::ostringstream name;

    name << action_name << "_";
    name.width(2);
    name.fill('0');
    name << _not_parsed_name_number++;

    _non_parsed.insert(name.str());
    reg<T>(name.str(), action_name);
  }
  
  Action * create(const std::string & name, InputParameters params);

  InputParameters getValidParams(const std::string & name);

  std::string isRegistered(const std::string & real_id, bool * is_parent = NULL);

  /**
   * This method will build all Actions associated with the passed action_name
   * if the parameters object doesn't contain any missing required parameters.
   * It returns true if at least one Action was built, therefore satisfying
   * the passed action_name
   */
  bool buildAllBuildableActions(const std::string & action_name, Parser * p_ptr);

  registeredActionIterator registeredActionsBegin() { return _name_to_action_map.begin(); }
  registeredActionIterator registeredActionsEnd() { return _name_to_action_map.end(); }

  bool isParsed(const std::string & name) const { return _non_parsed.find(name) == _non_parsed.end(); }
  
protected:
  std::map<std::string, buildActionPtr>  _name_to_build_pointer;
  std::map<std::string, paramsActionPtr> _name_to_params_pointer;
  std::map<std::string, std::string> _name_to_action_map;
  std::set<std::string> _non_parsed;
  std::multimap<std::string, std::string> _action_to_name_map;

  std::vector<std::string> _registered_parser_block_names;
  std::vector<Action *> _active_parser_blocks;
  unsigned int _not_parsed_name_number;

private:
  // Private constructor for singleton pattern
  ActionFactory();
};

#endif /* ACTIONFACTORY_H_ */
