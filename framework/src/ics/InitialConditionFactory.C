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
InitialConditionFactory::getValidParams(std::string name)
{
  if( name_to_params_pointer.find(name) == name_to_params_pointer.end() )
    mooseError(std::string("A _") + name + "_ is not registered InitialCondition ");

  return name_to_params_pointer[name]();
}

InitialConditionNamesIterator
InitialConditionFactory::registeredInitialConditionsBegin()
{
  // Make sure the _registered_kernel_names are up to date
  _registered_initial_condition_names.clear();
  _registered_initial_condition_names.reserve(name_to_params_pointer.size());

  // build a vector of strings from the params pointer map
  for (std::map<std::string, InitialConditionParamsPtr>::iterator i = name_to_params_pointer.begin();
       i != name_to_params_pointer.end();
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
