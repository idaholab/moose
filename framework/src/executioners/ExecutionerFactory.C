#include "ExecutionerFactory.h"
#include <iostream>

ExecutionerFactory::ExecutionerFactory()
{}

ExecutionerFactory *
ExecutionerFactory::instance()
{
  static ExecutionerFactory * instance;
  if(!instance)
    instance=new ExecutionerFactory;
  return instance;
}

Executioner *
ExecutionerFactory::build(std::string ex_name,
                          std::string name,
                          MooseSystem & moose_system,
                          InputParameters parameters)
{
  return (*name_to_build_pointer[ex_name])(name, moose_system, parameters);
}

InputParameters
ExecutionerFactory::getValidParams(std::string name)
{
  if( name_to_params_pointer.find(name) == name_to_params_pointer.end() )
    mooseError(std::string("A _") + name + "_ is not registered Executioner ");

  return name_to_params_pointer[name]();
}

ExecutionerNamesIterator
ExecutionerFactory::registeredExecutionersBegin()
{
  // Make sure the _registered_kernel_names are up to date
  _registered_initial_condition_names.clear();
  _registered_initial_condition_names.reserve(name_to_params_pointer.size());

  // build a vector of strings from the params pointer map
  for (std::map<std::string, ExecutionerParamsPtr>::iterator i = name_to_params_pointer.begin();
       i != name_to_params_pointer.end();
       ++i)
  {
    _registered_initial_condition_names.push_back(i->first);
  }
  
  return _registered_initial_condition_names.begin();
}

ExecutionerNamesIterator
ExecutionerFactory::registeredExecutionersEnd()
{
  return _registered_initial_condition_names.end();
}
