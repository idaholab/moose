#ifndef EXECUTIONERFACTORY_H
#define EXECUTIONERFACTORY_H

#include "Executioner.h"
#include "Moose.h"

// System includes
#include <map>
#include <string>
#include <vector>

/**
 * Typedef to make things easier.
 */
typedef Executioner * (*ExecutionerBuildPtr)(std::string name, InputParameters parameters);

/**
 * Typedef to make things easier.
 */
typedef InputParameters (*ExecutionerParamsPtr)();

/**
 * Typedef to hide implementation details
 */
typedef std::map<std::string, Executioner *>::iterator ExecutionerIterator;

/**
 * Typedef to hide implementation details
 */
typedef std::vector<std::string>::iterator ExecutionerNamesIterator;

/**
 * Templated build function used for generating function pointers to build classes on demand.
 */
template<typename ExecutionerType>
Executioner * buildExecutioner(std::string name, InputParameters parameters)
{
  return new ExecutionerType(name, parameters);
}

/**
 * Responsible for building Executioners on demand and storing them for retrieval
 */
class ExecutionerFactory
{
public:
  static ExecutionerFactory * instance();
  
  template<typename ExecutionerType> 
  void registerExecutioner(std::string name)
  {
    name_to_build_pointer[name]=&buildExecutioner<ExecutionerType>;
    name_to_params_pointer[name]=&validParams<ExecutionerType>;
  }

  Executioner * build(std::string ex_name, std::string name, InputParameters parameters);
  
  InputParameters getValidParams(std::string name);
  
  ExecutionerNamesIterator registeredExecutionersBegin();
  ExecutionerNamesIterator registeredExecutionersEnd();

private:
  ExecutionerFactory();
  
  virtual ~ExecutionerFactory(){}

  std::map<std::string, ExecutionerBuildPtr> name_to_build_pointer;
  std::map<std::string, ExecutionerParamsPtr> name_to_params_pointer;

  std::vector<std::string> _registered_initial_condition_names;
};

#endif //EXECUTIONERFACTORY_H
