#include "DamperFactory.h"
#include "MooseSystem.h"

DamperFactory *
DamperFactory::instance()
{
  static DamperFactory * instance;
  if(!instance)
    instance=new DamperFactory;
    
  return instance;
}

InputParameters
DamperFactory::getValidParams(std::string name)
{
  if( _name_to_params_pointer.find(name) == _name_to_params_pointer.end() )
  {
    mooseError(std::string("A _") + name + "_ is not a registered Damper\n\n");
  }

  InputParameters params = _name_to_params_pointer[name]();
  return params;
}

DamperNamesIterator
DamperFactory::registeredDampersBegin()
{
  // Make sure the _registered_damper_names are up to date
  _registered_damper_names.clear();
  _registered_damper_names.reserve(_name_to_params_pointer.size());

  // build a vector of strings from the params pointer map
  for (std::map<std::string, damperParamsPtr>::iterator i = _name_to_params_pointer.begin();
       i != _name_to_params_pointer.end();
       ++i)
  {
    _registered_damper_names.push_back(i->first);
  }

  return _registered_damper_names.begin();
}

DamperNamesIterator
DamperFactory::registeredDampersEnd()
{
  return _registered_damper_names.end();
}


DamperFactory::DamperFactory()
{
}
  
DamperFactory:: ~DamperFactory() 
{}

