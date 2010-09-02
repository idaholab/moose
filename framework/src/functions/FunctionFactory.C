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

#include "FunctionFactory.h"
#include "MooseSystem.h"

FunctionFactory *
FunctionFactory::instance()
{
  static FunctionFactory * instance;
  if(!instance)
    instance = new FunctionFactory;
    
  return instance;
}

InputParameters
FunctionFactory::getValidParams(std::string name)
{
  if( _name_to_params_pointer.find(name) == _name_to_params_pointer.end() )
  {
    mooseError(std::string("A _") + name + "_ is not a registered Function\n\n");
  }

  InputParameters params = _name_to_params_pointer[name]();
  return params;
}

FunctionFactory::FunctionFactory()
{
}
  
FunctionFactory::~FunctionFactory() 
{
  //TODO wtf???
  {
    std::map<std::string, functionBuildPtr>::iterator i;
    for(i=_name_to_build_pointer.begin(); i!=_name_to_build_pointer.end(); ++i)
    {
      delete &i;
    }
  }

  {
    std::map<std::string, functionParamsPtr>::iterator i;
    for(i=_name_to_params_pointer.begin(); i!=_name_to_params_pointer.end(); ++i)
    {
      delete &i;
    }
  }
}

FunctionNamesIterator
FunctionFactory::registeredFunctionsBegin()
{
  // Make sure the _registered_function_names are up to date
  _registered_function_names.clear();
  _registered_function_names.reserve(_name_to_params_pointer.size());

  // build a vector of strings from the params pointer map
  for (std::map<std::string, functionParamsPtr>::iterator i = _name_to_params_pointer.begin();
       i != _name_to_params_pointer.end();
       ++i)
  {
    _registered_function_names.push_back(i->first);
  }

  return _registered_function_names.begin();
}

FunctionNamesIterator
FunctionFactory::registeredFunctionsEnd()
{
  return _registered_function_names.end();
}

