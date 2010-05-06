#include "BCFactory.h"

BCFactory *
BCFactory::instance()
{
  static BCFactory * instance;
  if(!instance)
    instance=new BCFactory;
  return instance;
}

InputParameters
BCFactory::getValidParams(std::string name)
{
  if( name_to_params_pointer.find(name) == name_to_params_pointer.end() )
  {
    std::cerr<<std::endl<<"A _"<<name<<"_ is not a registered BC "<<std::endl<<std::endl;
    mooseError("");
  }

  return name_to_params_pointer[name]();
}

BCFactory::BCFactory()
{
}

BCFactory::~BCFactory()
{
  {
    std::map<std::string, BCBuildPtr>::iterator i;
    for (i=name_to_build_pointer.begin(); i!=name_to_build_pointer.end(); ++i)
    {
      delete &i;
    }
  }

  {
    std::map<std::string, BCParamsPtr>::iterator i;
    for(i=name_to_params_pointer.begin(); i!=name_to_params_pointer.end(); ++i)
    {
      delete &i;
    }

  }
}

BCNamesIterator
BCFactory::registeredBCsBegin()
{
  // Make sure the _registered_kernel_names are up to date
  _registered_bc_names.clear();
  _registered_bc_names.reserve(name_to_params_pointer.size());

  // build a vector of strings from the params pointer map
  for (std::map<std::string, BCParamsPtr>::iterator i = name_to_params_pointer.begin();
       i != name_to_params_pointer.end();
       ++i)
  {
    _registered_bc_names.push_back(i->first);
  }

  return _registered_bc_names.begin();
}

BCNamesIterator
BCFactory::registeredBCsEnd()
{
  return _registered_bc_names.end();
}

