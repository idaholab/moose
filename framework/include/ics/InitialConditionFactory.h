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
    if (_name_to_build_pointer.find(name) == _name_to_build_pointer.end())
    {
      _name_to_build_pointer[name] = &buildInitialCondition<InitialConditionType>;
      _name_to_params_pointer[name] = &validParams<InitialConditionType>;
    }
    else
      mooseError("InitialCondition '" + name + "' already registered.");
  }

  InitialCondition *create(std::string ic_name, std::string name, MooseSystem & moose_system, InputParameters parameters)
  {
    return (*_name_to_build_pointer[ic_name])(name, moose_system, parameters);
  }
  
  InputParameters getValidParams(std::string name);
  
  InitialConditionNamesIterator registeredInitialConditionsBegin();
  InitialConditionNamesIterator registeredInitialConditionsEnd();

private:
  InitialConditionFactory();
  
  virtual ~InitialConditionFactory(){}

  std::map<std::string, InitialConditionBuildPtr> _name_to_build_pointer;
  std::map<std::string, InitialConditionParamsPtr> _name_to_params_pointer;

  std::vector<std::string> _registered_initial_condition_names;
};

#endif //INITIALCONDITIONFACTORY_H
