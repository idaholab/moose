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
BCFactory::getValidParams(const std::string & name)
{
  if( _name_to_params_pointer.find(name) == _name_to_params_pointer.end() )
    mooseError("A _" + name + "_ is not a registered BC\n\n");
  
  return _name_to_params_pointer[name]();
}

BCFactory::BCFactory()
{
}

BCFactory::~BCFactory()
{
  {
    std::map<std::string, BCBuildPtr>::iterator i;
    for (i=_name_to_build_pointer.begin(); i!=_name_to_build_pointer.end(); ++i)
    {
      delete &i;
    }
  }

  {
    std::map<std::string, BCParamsPtr>::iterator i;
    for(i=_name_to_params_pointer.begin(); i!=_name_to_params_pointer.end(); ++i)
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
  _registered_bc_names.reserve(_name_to_params_pointer.size());

  // build a vector of strings from the params pointer map
  for (std::map<std::string, BCParamsPtr>::iterator i = _name_to_params_pointer.begin();
       i != _name_to_params_pointer.end();
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

