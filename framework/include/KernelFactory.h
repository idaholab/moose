#include "Kernel.h"

// System includes
#include <map>
#include <string>
#include <vector>

// LibMesh includes
#include <parameters.h>

#ifndef KERNELFACTORY_H
#define KERNELFACTORY_H

/**
 * Typedef to make things easier.
 */
typedef Kernel * (*kernelBuildPtr)(Parameters parameters,
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
Kernel * buildKernel(Parameters parameters,
                     std::string var_name,
                     std::vector<std::string> coupled_to,
                     std::vector<std::string> coupled_as)
{
  return new KernelType(parameters, var_name, coupled_to, coupled_as);
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

  void add(std::string name,
           Parameters parameters,
           std::string var_name,
           std::vector<std::string> coupled_to=std::vector<std::string>(0),
           std::vector<std::string> coupled_as=std::vector<std::string>(0))
  {
    active_kernels.push_back((*name_to_build_pointer[name])(parameters,var_name,coupled_to,coupled_as));
  }

  Parameters getValidParams(std::string name)
  {
    if( name_to_params_pointer.find(name) == name_to_params_pointer.end() )
    {
      std::cerr<<std::endl<<"A _"<<name<<"_ is not registered Kernel "<<std::endl<<std::endl;
      error();
    }
    return name_to_params_pointer[name]();
  }

  std::vector<Kernel *>::iterator activeKernelsBegin(){ return active_kernels.begin(); };
  std::vector<Kernel *>::iterator activeKernelsEnd(){ return active_kernels.end(); };

private:
  KernelFactory(){}
  virtual ~KernelFactory(){}

  std::map<std::string, kernelBuildPtr> name_to_build_pointer;
  std::map<std::string, kernelParamsPtr> name_to_params_pointer;

  std::vector<Kernel *> active_kernels;
};

#endif //KERNELFACTORY_H
