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
                          const std::string & name,
                          InputParameters parameters)
{
  return (*_name_to_build_pointer[ex_name])(name, parameters);
}

InputParameters
ExecutionerFactory::getValidParams(const std::string & name)
{
  if( _name_to_params_pointer.find(name) == _name_to_params_pointer.end() )
    mooseError(std::string("A _") + name + "_ is not registered Executioner ");

  return _name_to_params_pointer[name]();
}

ExecutionerNamesIterator
ExecutionerFactory::registeredExecutionersBegin()
{
  // Make sure the _registered_kernel_names are up to date
  _registered_initial_condition_names.clear();
  _registered_initial_condition_names.reserve(_name_to_params_pointer.size());

  // build a vector of strings from the params pointer map
  for (std::map<std::string, ExecutionerParamsPtr>::iterator i = _name_to_params_pointer.begin();
       i != _name_to_params_pointer.end();
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
