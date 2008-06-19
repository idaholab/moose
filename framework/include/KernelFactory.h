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
typedef Kernel * (*kernelBuildPtr)(Parameters parameters, EquationSystems * es, std::string var_name);

/**
 * Templated build function used for generating function pointers to build classes on demand.
 */
template<typename KernelType>
Kernel * buildKernel(Parameters parameters, EquationSystems * es, std::string var_name)
{
  return new KernelType(parameters, es, var_name);
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
  }

  void add(std::string name, Parameters parameters, EquationSystems * es, std::string var_name)
  {
    active_kernels.push_back((*name_to_build_pointer[name])(parameters,es,var_name));
  }

  std::vector<Kernel *>::iterator activeKernelsBegin(){ return active_kernels.begin(); };
  std::vector<Kernel *>::iterator activeKernelsEnd(){ return active_kernels.end(); };

private:
  KernelFactory(){}
  virtual ~KernelFactory(){}

  std::map<std::string, kernelBuildPtr> name_to_build_pointer;
  std::vector<Kernel *> active_kernels;
};

#endif //KERNELFACTORY_H
