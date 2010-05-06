#include "AuxFactory.h"
#include <list> 

AuxFactory *
AuxFactory::instance()
  {
    static AuxFactory * instance;
    if(!instance)
      instance=new AuxFactory;
    return instance;
  }

InputParameters
AuxFactory::getValidParams(std::string name)
  {
    if( name_to_params_pointer.find(name) == name_to_params_pointer.end() )
    {

      std::cerr<<std::endl<<"A _"<<name<<"_ is not a registered Aux "<<std::endl<<std::endl;
      mooseError("");

    }
    return name_to_params_pointer[name]();
  }

  
AuxFactory::AuxFactory()
{
}

AuxFactory:: ~AuxFactory()
{
  {
    std::map<std::string, AuxKernelBuildPtr>::iterator i;
    for(i=name_to_build_pointer.begin(); i!=name_to_build_pointer.end(); ++i)
    {
      delete &i;
    }
  }

  {
    std::map<std::string, AuxKernelParamsPtr>::iterator i;
    for(i=name_to_params_pointer.begin(); i!=name_to_params_pointer.end(); ++i)
    {
      delete &i;
    }
  }
}

AuxKernelNamesIterator
AuxFactory::registeredAuxKernelsBegin()
{
  // Make sure the _registered_auxkernel_names are up to date
  _registered_auxkernel_names.clear();
  _registered_auxkernel_names.reserve(AuxFactory::instance()->name_to_params_pointer.size());

  // build a vector of strings from the params pointer map
  for (std::map<std::string, AuxKernelParamsPtr>::iterator i = name_to_params_pointer.begin();
       i != name_to_params_pointer.end();
       ++i)
  {
    _registered_auxkernel_names.push_back(i->first);
  }

  return _registered_auxkernel_names.begin();
}

AuxKernelNamesIterator
AuxFactory::registeredAuxKernelsEnd()
{
  return _registered_auxkernel_names.end();
}
