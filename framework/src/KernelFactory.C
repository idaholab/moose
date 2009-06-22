#include "KernelFactory.h"

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
  
