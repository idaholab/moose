#include "KernelFactory.h"
#include "MooseSystem.h"

KernelFactory *
KernelFactory::instance()
{
  static KernelFactory * instance;
  if(!instance)
    instance=new KernelFactory;
    
  return instance;
}

InputParameters
KernelFactory::getValidParams(std::string name)
{
  if( name_to_params_pointer.find(name) == name_to_params_pointer.end() )
  {
    mooseError(std::string("A _") + name + "_ is not a registered Kernel\n\n");
  }

  InputParameters params = name_to_params_pointer[name]();

  if(!params.have_parameter<Real>("start_time"))
    params.addParam<Real>("start_time", -std::numeric_limits<Real>::max(), "The time that this kernel will be active after.");

  if(!params.have_parameter<Real>("stop_time"))
    params.addParam<Real>("stop_time", std::numeric_limits<Real>::max(), "The time after which this kernel will no longer be active.");

  return params;
}

KernelNamesIterator
KernelFactory::registeredKernelsBegin()
{
  // Make sure the _registered_kernel_names are up to date
  _registered_kernel_names.clear();
  _registered_kernel_names.reserve(name_to_params_pointer.size());

  // build a vector of strings from the params pointer map
  for (std::map<std::string, kernelParamsPtr>::iterator i = name_to_params_pointer.begin();
       i != name_to_params_pointer.end();
       ++i)
  {
    _registered_kernel_names.push_back(i->first);
  }

  return _registered_kernel_names.begin();
}

KernelNamesIterator
KernelFactory::registeredKernelsEnd()
{
  return _registered_kernel_names.end();
}


KernelFactory::KernelFactory()
{
}
  
KernelFactory:: ~KernelFactory() 
{
  {
    std::map<std::string, kernelBuildPtr>:: iterator i;
    for(i=name_to_build_pointer.begin(); i!=name_to_build_pointer.end(); ++i)
    {
      delete &i;
    }
  }

  {
    std::map<std::string, kernelParamsPtr>::iterator i;
    for(i=name_to_params_pointer.begin(); i!=name_to_params_pointer.end(); ++i)
    {
      delete &i;
    }
  }
}

