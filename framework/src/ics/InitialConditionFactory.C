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

#include "InitialConditionFactory.h"
#include <iostream>

InitialConditionFactory *
InitialConditionFactory::instance()
{
  static InitialConditionFactory * instance;
  if(!instance)
    instance=new InitialConditionFactory;
  return instance;
}

InputParameters
InitialConditionFactory::getValidParams(const std::string & name)
{
  if( _name_to_params_pointer.find(name) == _name_to_params_pointer.end() )
    mooseError(std::string("A _") + name + "_ is not registered InitialCondition ");

  return _name_to_params_pointer[name]();
}

InitialConditionNamesIterator
InitialConditionFactory::registeredInitialConditionsBegin()
{
  // Make sure the _registered_kernel_names are up to date
  _registered_initial_condition_names.clear();
  _registered_initial_condition_names.reserve(_name_to_params_pointer.size());

  // build a vector of strings from the params pointer map
  for (std::map<std::string, InitialConditionParamsPtr>::iterator i = _name_to_params_pointer.begin();
       i != _name_to_params_pointer.end();
       ++i)
  {
    _registered_initial_condition_names.push_back(i->first);
  }
  
  return _registered_initial_condition_names.begin();
}

InitialConditionNamesIterator
InitialConditionFactory::registeredInitialConditionsEnd()
{
  return _registered_initial_condition_names.end();
}

InitialConditionFactory::InitialConditionFactory()
{
}
