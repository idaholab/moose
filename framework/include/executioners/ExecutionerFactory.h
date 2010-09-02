/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Orieneted Simulation Environment */
/*                                                              */
/*            @ 2010 Battelle Energy Alliance, LLC              */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
typedef Executioner * (*ExecutionerBuildPtr)(std::string name, MooseSystem & moose_system, InputParameters parameters);

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
Executioner * buildExecutioner(std::string name, MooseSystem & moose_system, InputParameters parameters)
{
  return new ExecutionerType(name, moose_system, parameters);
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
    if (_name_to_build_pointer.find(name) == _name_to_build_pointer.end())
    {
      _name_to_build_pointer[name] = &buildExecutioner<ExecutionerType>;
      _name_to_params_pointer[name] = &validParams<ExecutionerType>;
    }
    else
      mooseError("Executioner '" + name + "' already registered.");
  }

  Executioner * build(std::string ex_name, std::string name, MooseSystem & moose_system, InputParameters parameters);
  
  InputParameters getValidParams(std::string name);
  
  ExecutionerNamesIterator registeredExecutionersBegin();
  ExecutionerNamesIterator registeredExecutionersEnd();

private:
  ExecutionerFactory();
  
  virtual ~ExecutionerFactory(){}

  std::map<std::string, ExecutionerBuildPtr> _name_to_build_pointer;
  std::map<std::string, ExecutionerParamsPtr> _name_to_params_pointer;

  std::vector<std::string> _registered_initial_condition_names;
};

#endif //EXECUTIONERFACTORY_H
