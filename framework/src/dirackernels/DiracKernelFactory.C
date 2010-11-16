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

#include "DiracKernelFactory.h"
#include "MooseSystem.h"

DiracKernelFactory *
DiracKernelFactory::instance()
{
  static DiracKernelFactory * instance;
  if(!instance)
    instance=new DiracKernelFactory;
    
  return instance;
}

InputParameters
DiracKernelFactory::getValidParams(const std::string & name)
{
  if( _name_to_params_pointer.find(name) == _name_to_params_pointer.end() )
  {
    mooseError(std::string("A _") + name + "_ is not a registered DiracKernel\n\n");
  }

  InputParameters params = _name_to_params_pointer[name]();
  return params;
}

DiracKernelNamesIterator
DiracKernelFactory::registeredDiracKernelsBegin()
{
  // Make sure the _registered_dirac_kernel_names are up to date
  _registered_dirac_kernel_names.clear();
  _registered_dirac_kernel_names.reserve(_name_to_params_pointer.size());

  // build a vector of strings from the params pointer map
  for (std::map<std::string, dirac_kernelParamsPtr>::iterator i = _name_to_params_pointer.begin();
       i != _name_to_params_pointer.end();
       ++i)
  {
    _registered_dirac_kernel_names.push_back(i->first);
  }

  return _registered_dirac_kernel_names.begin();
}

DiracKernelNamesIterator
DiracKernelFactory::registeredDiracKernelsEnd()
{
  return _registered_dirac_kernel_names.end();
}


DiracKernelFactory::DiracKernelFactory()
{
}
  
DiracKernelFactory:: ~DiracKernelFactory() 
{}

