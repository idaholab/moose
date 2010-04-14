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

void
InitialConditionFactory::add(std::string ic_name,
                             std::string name,
                             MooseSystem & moose_system,
                             InputParameters parameters,
                             std::string var_name)
{
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    Moose::current_thread_id = tid;

    // The var_name needs to be added to the parameters object for any InitialCondition derived objects
    parameters.set<std::string>("var_name") = var_name;
    
    active_initial_conditions[tid][var_name] = (*name_to_build_pointer[ic_name])(name, moose_system, parameters);
  }
}

InputParameters
InitialConditionFactory::getValidParams(std::string name)
{
  if( name_to_params_pointer.find(name) == name_to_params_pointer.end() )
    mooseError(std::string("A _") + name + "_ is not registered InitialCondition ");

  return name_to_params_pointer[name]();
}

InitialCondition *
InitialConditionFactory::getInitialCondition(THREAD_ID tid, std::string var_name)
{
  InitialConditionIterator ic_iter = active_initial_conditions[tid].find(var_name);

  if (ic_iter == active_initial_conditions[tid].end()) 
    return NULL;

  return ic_iter->second;
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


InitialConditionIterator
InitialConditionFactory::activeInitialConditionsBegin(THREAD_ID tid)
{
  return active_initial_conditions[tid].begin();
}

InitialConditionIterator
InitialConditionFactory::activeInitialConditionsEnd(THREAD_ID tid)
{
  return active_initial_conditions[tid].end();
}

InitialConditionFactory::InitialConditionFactory()
{
  active_initial_conditions.resize(libMesh::n_threads());
}
