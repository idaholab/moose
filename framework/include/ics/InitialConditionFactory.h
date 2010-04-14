#ifndef INITIALCONDITIONFACTORY_H
#define INITIALCONDITIONFACTORY_H

#include "InitialCondition.h"
#include "Kernel.h"

// System includes
#include <map>
#include <string>
#include <vector>

// LibMesh includes
#include <parameters.h>

class MooseSystem;

/**
 * Typedef to make things easier.
 */
typedef InitialCondition * (*InitialConditionBuildPtr)(std::string name, MooseSystem & moose_system, InputParameters parameters);

/**
 * Typedef to make things easier.
 */
typedef InputParameters (*InitialConditionParamsPtr)();

/**
 * Typedef to hide implementation details
 */
typedef std::map<std::string, InitialCondition *>::iterator InitialConditionIterator;

/**
 * Typedef to hide implementation details
 */
typedef std::vector<std::string>::iterator InitialConditionNamesIterator;

/**
 * Templated build function used for generating function pointers to build classes on demand.
 */
template<typename InitialConditionType>
InitialCondition * buildInitialCondition(std::string name,
                                         MooseSystem & moose_system,
                                         InputParameters parameters)
{
  return new InitialConditionType(name, moose_system, parameters);
}

/**
 * Responsible for building InitialConditions on demand and storing them for retrieval
 */
class InitialConditionFactory
{
public:
  static InitialConditionFactory * instance();
  
  template<typename InitialConditionType> 
  void registerInitialCondition(std::string name)
  {
    name_to_build_pointer[name]=&buildInitialCondition<InitialConditionType>;
    name_to_params_pointer[name]=&validParams<InitialConditionType>;
  }

  void add(std::string ic_name, std::string name, MooseSystem & moose_system, InputParameters parameters, std::string var_name);
  
  InputParameters getValidParams(std::string name);
  
  InitialCondition * getInitialCondition(THREAD_ID tid, std::string var_name);
  
  InitialConditionIterator activeInitialConditionsBegin(THREAD_ID tid);
  InitialConditionIterator activeInitialConditionsEnd(THREAD_ID tid);

  InitialConditionNamesIterator registeredInitialConditionsBegin();
  InitialConditionNamesIterator registeredInitialConditionsEnd();

private:
  InitialConditionFactory();
  
  virtual ~InitialConditionFactory(){}

  std::map<std::string, InitialConditionBuildPtr> name_to_build_pointer;
  std::map<std::string, InitialConditionParamsPtr> name_to_params_pointer;

  std::vector<std::string> _registered_initial_condition_names;
  std::vector<std::map<std::string, InitialCondition *> > active_initial_conditions;
};

#endif //INITIALCONDITIONFACTORY_H
