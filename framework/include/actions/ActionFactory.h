#ifndef ACTIONFACTORY_H_
#define ACTIONFACTORY_H_

#include <vector>

#include "Moose.h"
#include "Action.h"
#include "InputParameters.h"

/**
 * Macros
 */
#define stringifyName(name) #name
#define registerAction(tplt, name, action)      ActionFactory::instance()->reg<tplt>(name, action)


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
    }
    else
      mooseError("Action '" + name + "' already registered.");
  }

  Action * add(const std::string & name, InputParameters params);

  InputParameters getValidParams(const std::string & name);

  std::string isRegistered(const std::string & real_id);

protected:
  std::map<std::string, buildActionPtr>  _name_to_build_pointer;
  std::map<std::string, paramsActionPtr> _name_to_params_pointer;
  std::map<std::string, std::string> _name_to_action_map;

  std::vector<std::string> _registered_parser_block_names;
  std::vector<Action *> _active_parser_blocks;
};

#endif /* ACTIONFACTORY_H_ */
