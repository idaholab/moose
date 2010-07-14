#ifndef FUNCTIONFACTORY_H
#define FUNCTIONFACTORY_H

#include "Function.h"

// System includes
#include <map>
#include <string>
#include <vector>
#include <typeinfo>

// LibMesh includes
#include <parameters.h>

// forward declarations
class MooseSystem;

/**
 * Typedef to make things easier.
 */
typedef Function * (*functionBuildPtr)(std::string name, MooseSystem & moose_system, InputParameters parameters);

/**
 * Typedef to hide implementation details
 */
typedef std::vector<std::string>::iterator FunctionNamesIterator;

/**
 * Typedef to make things easier.
 */
typedef InputParameters (*functionParamsPtr)();

/**
 * Templated build function used for generating function pointers to build classes on demand.
 */
template<typename FunctionType>
Function * buildFunction(std::string name, MooseSystem & moose_system, InputParameters parameters)
{
  return new FunctionType(name, moose_system, parameters);
}

/**
 * Responsible for building Functions on demand and storing them for retrieval
 * PJJ TODO: clean this class up. it has unecessary cruft from KernelFactory
 */
class FunctionFactory
{
public:
  static FunctionFactory * instance();

  template<typename FunctionType> 
  void registerFunction(std::string name)
  {
    if (_name_to_build_pointer.find(name) == _name_to_build_pointer.end())
    {
      _name_to_build_pointer[name] = &buildFunction<FunctionType>;
      _name_to_params_pointer[name] = &validParams<FunctionType>;
    }
    else
      mooseError("Function '" + name + "' already registered.");
  }

  Function *create(std::string function_name, std::string name, MooseSystem & moose_system, InputParameters parameters)
  {
    return (*_name_to_build_pointer[function_name])(name, moose_system, parameters);
  }

  FunctionNamesIterator registeredFunctionsBegin();
  FunctionNamesIterator registeredFunctionsEnd();

  InputParameters getValidParams(std::string name);
  
private:
  FunctionFactory();
  virtual ~FunctionFactory();
  
  std::map<std::string, functionBuildPtr> _name_to_build_pointer;
  std::map<std::string, functionParamsPtr> _name_to_params_pointer;

  std::vector<std::string> _registered_function_names;
};

#endif //FUNCTIONFACTORY_H
