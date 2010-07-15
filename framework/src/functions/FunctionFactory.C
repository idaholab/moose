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
