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
    if( _name_to_params_pointer.find(name) == _name_to_params_pointer.end() )
      mooseError("\nA _" + name + "_ is not a registered Aux\n\n");
    
    return _name_to_params_pointer[name]();
  }

  
AuxFactory::AuxFactory()
{
}

AuxFactory:: ~AuxFactory()
{}

AuxKernelNamesIterator
AuxFactory::registeredAuxKernelsBegin()
{
  // Make sure the _registered_auxkernel_names are up to date
  _registered_auxkernel_names.clear();
  _registered_auxkernel_names.reserve(AuxFactory::instance()->_name_to_params_pointer.size());

  // build a vector of strings from the params pointer map
  for (std::map<std::string, AuxKernelParamsPtr>::iterator i = _name_to_params_pointer.begin();
       i != _name_to_params_pointer.end();
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
