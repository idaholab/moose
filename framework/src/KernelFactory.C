#include "KernelFactory.h"

KernelFactory *
KernelFactory::instance()
{
  static KernelFactory * instance;
  if(!instance)
    instance=new KernelFactory;
    
  return instance;
}

Kernel *
KernelFactory::add(std::string kernel_name,
                   std::string name,
                   Parameters parameters,
                   std::string var_name,
                   std::vector<std::string> coupled_to,
                   std::vector<std::string> coupled_as)
{
  Kernel * kernel;
    
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    Moose::current_thread_id = tid;

    kernel = (*name_to_build_pointer[kernel_name])(name,parameters,var_name,coupled_to,coupled_as);

    active_kernels[tid].push_back(kernel);
  }

  return kernel;
}

Kernel *
KernelFactory::add(std::string kernel_name,
                   std::string name,
                   Parameters parameters,
                   std::string var_name,
                   std::vector<std::string> coupled_to,
                   std::vector<std::string> coupled_as,
                   unsigned int block_id)
{
  Kernel * kernel;
    
  for(THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    Moose::current_thread_id = tid;
      
    kernel = (*name_to_build_pointer[kernel_name])(name,parameters,var_name,coupled_to,coupled_as);

    block_kernels[tid][block_id].push_back(kernel);
  }

  return kernel;
}

Parameters
KernelFactory::getValidParams(std::string name)
{
  if( name_to_params_pointer.find(name) == name_to_params_pointer.end() )
  {
    std::cerr<<std::endl<<"A _"<<name<<"_ is not a registered Kernel "<<std::endl<<std::endl;
    error();
  }
  return name_to_params_pointer[name]();
}

KernelIterator
KernelFactory::activeKernelsBegin(THREAD_ID tid)
{
  return active_kernels[tid].begin();
}

KernelIterator
KernelFactory::activeKernelsEnd(THREAD_ID tid)
{
  return active_kernels[tid].end();
}


KernelIterator
KernelFactory::blockKernelsBegin(THREAD_ID tid, unsigned int block_id)
{
  return block_kernels[tid][block_id].begin();
}

KernelIterator
KernelFactory::blockKernelsEnd(THREAD_ID tid, unsigned int block_id)
{
  return block_kernels[tid][block_id].end();
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
  active_kernels.resize(libMesh::n_threads());
  block_kernels.resize(libMesh::n_threads());
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
     
  {
        
    std::vector<std::vector<Kernel *> >::iterator i;
    for (i=active_kernels.begin(); i!=active_kernels.end(); ++i)
    { 

      KernelIterator j;
      for (j=i->begin(); j!=i->end(); ++j)
      {
        delete *j;
      }
    }
  }

  {
    std::vector<std::map<unsigned int, std::vector<Kernel *> > >::iterator i;
    for (i=block_kernels.begin(); i!=block_kernels.end(); ++i)
    {
          
      std::map<unsigned int, std::vector<Kernel *> >::iterator j;
      for (j=i->begin(); j!=i->end(); ++j)
      {
        KernelIterator k;
        for(k=(j->second).begin(); k!=(j->second).end(); ++k)
        {
          delete *k;
        }
      }
    }
  }
}

