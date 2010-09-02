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

#include "DGKernelFactory.h"
#include "MooseSystem.h"

DGKernelFactory *
DGKernelFactory::instance()
{
  static DGKernelFactory * instance;
  if(!instance)
    instance=new DGKernelFactory;
    
  return instance;
}

InputParameters
DGKernelFactory::getValidParams(std::string name)
{
  if( _name_to_params_pointer.find(name) == _name_to_params_pointer.end() )
  {
    mooseError("A _" + name + "_ is not a registered DGKernel\n\n");
  }

  InputParameters params = _name_to_params_pointer[name]();
  return params;
}

DGKernelNamesIterator
DGKernelFactory::registeredDGKernelsBegin()
{
  // Make sure the _registered_kernel_names are up to date
  _registered_dgkernel_names.clear();
  _registered_dgkernel_names.reserve(_name_to_params_pointer.size());

  // build a vector of strings from the params pointer map
  for (std::map<std::string, dgKernelParamsPtr>::iterator i = _name_to_params_pointer.begin();
       i != _name_to_params_pointer.end();
       ++i)
  {
    _registered_dgkernel_names.push_back(i->first);
  }

  return _registered_dgkernel_names.begin();
}

DGKernelNamesIterator
DGKernelFactory::registeredDGKernelsEnd()
{
  return _registered_dgkernel_names.end();
}


DGKernelFactory::DGKernelFactory()
{
}
  
DGKernelFactory:: ~DGKernelFactory()
{
  {
    std::map<std::string, dgKernelBuildPtr>:: iterator i;
    for(i=_name_to_build_pointer.begin(); i!=_name_to_build_pointer.end(); ++i)
    {
      delete &i;
    }
  }

  {
    std::map<std::string, dgKernelParamsPtr>::iterator i;
    for(i=_name_to_params_pointer.begin(); i!=_name_to_params_pointer.end(); ++i)
    {
      delete &i;
    }
  }
}

