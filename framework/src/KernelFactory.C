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

std::vector<Kernel *>::iterator
KernelFactory::activeKernelsBegin(THREAD_ID tid)
{
  return active_kernels[tid].begin();
}

std::vector<Kernel *>::iterator
KernelFactory::activeKernelsEnd(THREAD_ID tid)
{
  return active_kernels[tid].end();
}


std::vector<Kernel *>::iterator
KernelFactory::blockKernelsBegin(THREAD_ID tid, unsigned int block_id)
{
  return block_kernels[tid][block_id].begin();
}

std::vector<Kernel *>::iterator
KernelFactory::blockKernelsEnd(THREAD_ID tid, unsigned int block_id)
{
  return block_kernels[tid][block_id].end();
}


KernelFactory::KernelFactory()
  {
    active_kernels.resize(libMesh::n_threads());
    block_kernels.resize(libMesh::n_threads());
  }
  
