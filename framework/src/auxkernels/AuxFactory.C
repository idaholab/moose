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
AuxFactory::getValidParams(const std::string & name)
  {
    if( _name_to_params_pointer.find(name) == _name_to_params_pointer.end() )
      mooseError("\nA _" + name + "_ is not a registered Aux\n\n");
    
    return _name_to_params_pointer[name]();
  }

  
AuxFactory::AuxFactory()
{
}

AuxFactory:: ~AuxFactory()
{
  {
    std::map<std::string, AuxKernelBuildPtr>::iterator i;
    for(i=_name_to_build_pointer.begin(); i!=_name_to_build_pointer.end(); ++i)
    {
      delete &i;
    }
  }

  {
    std::map<std::string, AuxKernelParamsPtr>::iterator i;
    for(i=_name_to_params_pointer.begin(); i!=_name_to_params_pointer.end(); ++i)
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
