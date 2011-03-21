#ifndef ACTIONFACTORY_H_
#define ACTIONFACTORY_H_

#include <vector>

#include "Moose.h"
#include "Action.h"
#include "InputParameters.h"
#include "ActionWarehouse.h"

/**
 * Macros
 */
#define stringifyName(name) #name
#define registerActionName(name, is_required)   action_warehouse.registerName(name, is_required)
#define registerAction(tplt, name, action)      ActionFactory::instance()->reg<tplt>(name, action)
#define registerNonParsedAction(tplt, action)   ActionFactory::instance()->regNonParsed<tplt>(action)


/**
 * Typedef for function to build objects
 */
typedef Action * (*buildActionPtr)(const std::string & name, InputParameters parameters);

/**
 * Typedef for validParams
 */
typedef InputParameters (*paramsActionPtr)();


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
    static unsigned int not_parsed_name_number = 0;
    OStringStream name;

    name << "non_parsed";
    OSSRealzeroleft(name,2,0,not_parsed_name_number);
    
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

protected:
  std::map<std::string, buildActionPtr>  _name_to_build_pointer;
  std::map<std::string, paramsActionPtr> _name_to_params_pointer;
  std::map<std::string, std::string> _name_to_action_map;
  std::multimap<std::string, std::string> _action_to_name_map;

  std::vector<std::string> _registered_parser_block_names;
  std::vector<Action *> _active_parser_blocks;

private:
  // Private constructor for singleton pattern
  ActionFactory() {}
};

#endif /* ACTIONFACTORY_H_ */
