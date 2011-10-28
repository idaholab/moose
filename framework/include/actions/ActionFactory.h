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

#ifndef ACTIONFACTORY_H
#define ACTIONFACTORY_H

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
#define registerAction(tplt, action)      ActionFactory::instance()->reg<tplt>(stringifyName(tplt), action)

// TODO: This will change when action_warehouse is moved inside of some system
#define addActionNameDependency(action, depends_on)       Moose::action_warehouse.addDependency(action, depends_on)

// Forward Declaration
class ActionFactory;

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
 * Specialized factory for generic Action System objects
 */
class ActionFactory
{
public:
  static ActionFactory *instance();

  static void release();

  virtual ~ActionFactory();

  template<typename T>
  void reg(const std::string & name, const std::string & action_name)
  {
    BuildInfo build_info;
    build_info._build_pointer = &buildAction<T>;
    build_info._params_pointer = &validParams<T>;
    build_info._action_name = action_name;
    build_info._unique_id = _unique_id++;
    _name_to_build_info.insert(std::make_pair(name, build_info));

    _action_to_name_map.insert(std::make_pair(action_name, name));
  }

  std::string getActionName(const std::string & action);

  Action * create(const std::string & action, InputParameters params);

  InputParameters getValidParams(const std::string & name);

  /**
   * This method will build all Actions associated with the passed action_name
   * if the parameters object doesn't contain any missing required parameters.
   * It returns true if at least one Action was built, therefore satisfying
   * the passed action_name
   */
  bool buildAllBuildableActions(const std::string & action_name, Parser * p_ptr);

  class BuildInfo
  {
  public:
    buildActionPtr _build_pointer;
    paramsActionPtr _params_pointer;
    std::string _action_name;
    unsigned int _unique_id;
  };

  /**
   * Typedef for registered Action iterator
   **/
  typedef std::multimap<std::string, BuildInfo>::iterator iterator;
  typedef std::multimap<std::string, BuildInfo>::const_iterator const_iterator;

  iterator begin();
  const_iterator begin() const;

  iterator end();
  const_iterator end() const;

  bool isParsed(const std::string & name) const;

  std::multimap<std::string, BuildInfo> _name_to_build_info;

  std::set<std::string> _non_parsed;
  std::multimap<std::string, std::string> _action_to_name_map;

  std::vector<std::string> _registered_parser_block_names;

  // TODO: I don't think we need this anymore
  static unsigned int _unique_id;        ///< Unique ID for identifing multiple registrations

  static ActionFactory *_instance;       ///< Pointer to the singleton instance

private:
  // Private constructor for singleton pattern
  ActionFactory() {}
};

// -----------------------------------------------------------------------------
// ActionFactory class inline methods
inline
ActionFactory::iterator ActionFactory::begin()
{
  return _name_to_build_info.begin();
}

inline
ActionFactory::const_iterator ActionFactory::begin() const
{
  return _name_to_build_info.begin();
}

inline
ActionFactory::iterator ActionFactory::end()
{
  return _name_to_build_info.end();
}

inline
ActionFactory::const_iterator ActionFactory::end() const
{
  return _name_to_build_info.end();
}

inline
bool ActionFactory::isParsed(const std::string & name) const
{
  return _non_parsed.find(name) == _non_parsed.end();
}

#endif /* ACTIONFACTORY_H */
