#include "Kernel.h"

// System includes
#include <map>
#include <string>
#include <vector>
#include <typeinfo>

// LibMesh includes
#include <parameters.h>

#ifndef KERNELFACTORY_H
#define KERNELFACTORY_H

/**
 * Typedef to make things easier.
 */
typedef Kernel * (*kernelBuildPtr)(std::string name,
                                   Parameters parameters,
                                   std::string var_name,
                                   std::vector<std::string> coupled_to,
                                   std::vector<std::string> coupled_as);
/**
 * Typedef to make things easier.
 */
typedef Parameters (*kernelParamsPtr)();

/**
 * Templated build function used for generating function pointers to build classes on demand.
 */
template<typename KernelType>
Kernel * buildKernel(std::string name,
                     Parameters parameters,
                     std::string var_name,
                     std::vector<std::string> coupled_to,
                     std::vector<std::string> coupled_as)
{
  return new KernelType(name, parameters, var_name, coupled_to, coupled_as);
}

/**
 * Responsible for building Kernels on demand and storing them for retrieval
 */
class KernelFactory
{
public:
  static KernelFactory * instance()
  {
    static KernelFactory * instance;
    if(!instance)
      instance=new KernelFactory;
    return instance;
  }

  template<typename KernelType> 
  void registerKernel(std::string name)
  {
    name_to_build_pointer[name]=&buildKernel<KernelType>;
    name_to_params_pointer[name]=&valid_params<KernelType>;
  }

  Kernel * add(std::string kernel_name,
               std::string name,
               Parameters parameters,
               std::string var_name,
               std::vector<std::string> coupled_to=std::vector<std::string>(0),
               std::vector<std::string> coupled_as=std::vector<std::string>(0))
  {
    Kernel * kernel = (*name_to_build_pointer[kernel_name])(name,parameters,var_name,coupled_to,coupled_as);

    active_kernels.push_back(kernel);

    return kernel;
  }


  Kernel * add(std::string kernel_name,
               std::string name,
               Parameters parameters,
               std::string var_name,
               std::vector<std::string> coupled_to,
               std::vector<std::string> coupled_as,
               unsigned int block_id)
  {
    Kernel * kernel = (*name_to_build_pointer[kernel_name])(name,parameters,var_name,coupled_to,coupled_as);

    std::cout<<"Adding "<<kernel->name()<<" to block "<<block_id<<std::endl;

    block_kernels[block_id].push_back(kernel);

    return kernel;
  }

  Parameters getValidParams(std::string name)
  {
    if( name_to_params_pointer.find(name) == name_to_params_pointer.end() )
    {
      std::cerr<<std::endl<<"A _"<<name<<"_ is not a registered Kernel "<<std::endl<<std::endl;
      error();
    }
    return name_to_params_pointer[name]();
  }

  std::vector<Kernel *>::iterator activeKernelsBegin(){ return active_kernels.begin(); };
  std::vector<Kernel *>::iterator activeKernelsEnd(){ return active_kernels.end(); };

  std::vector<Kernel *>::iterator blockKernelsBegin(unsigned int block_id){
    std::cout<<"BK size "<<block_kernels[block_id].size()<<std::endl;
    return block_kernels[block_id].begin(); };
  std::vector<Kernel *>::iterator blockKernelsEnd(unsigned int block_id){ return block_kernels[block_id].end(); };

private:
  KernelFactory(){}
  virtual ~KernelFactory(){}

  std::map<std::string, kernelBuildPtr> name_to_build_pointer;
  std::map<std::string, kernelParamsPtr> name_to_params_pointer;

  std::vector<Kernel *> active_kernels;

  std::map<unsigned int, std::vector<Kernel *> > block_kernels;
};

#endif //KERNELFACTORY_H
