#ifndef KERNELFACTORY_H
#define KERNELFACTORY_H

#include "Kernel.h"

// System includes
#include <map>
#include <string>
#include <vector>
#include <typeinfo>

// LibMesh includes
#include <parameters.h>

// forward declarations
class MooseSystem;

/**
 * Typedef to make things easier.
 */
typedef Kernel * (*kernelBuildPtr)(std::string name, MooseSystem & moose_system, InputParameters parameters);

/**
 * Typedef to hide implementation details
 */
typedef std::vector<std::string>::iterator KernelNamesIterator;

/**
 * Typedef to make things easier.
 */
typedef InputParameters (*kernelParamsPtr)();

/**
 * Templated build function used for generating function pointers to build classes on demand.
 */
template<typename KernelType>
Kernel * buildKernel(std::string name, MooseSystem & moose_system, InputParameters parameters)
{
  return new KernelType(name, moose_system, parameters);
}

/**
 * Responsible for building Kernels on demand and storing them for retrieval
 */
class KernelFactory
{
public:
  static KernelFactory * instance();

  template<typename KernelType> 
  void registerKernel(std::string name)
  {
    _name_to_build_pointer[name]=&buildKernel<KernelType>;
    _name_to_params_pointer[name]=&validParams<KernelType>;
  }

  Kernel *create(std::string kernel_name, std::string name, MooseSystem & moose_system, InputParameters parameters)
  {
    return (*_name_to_build_pointer[kernel_name])(name, moose_system, parameters);
  }

  KernelNamesIterator registeredKernelsBegin();
  KernelNamesIterator registeredKernelsEnd();

  InputParameters getValidParams(std::string name);
  
private:
  KernelFactory();

  virtual ~KernelFactory();
  
  std::map<std::string, kernelBuildPtr> _name_to_build_pointer;
  std::map<std::string, kernelParamsPtr> _name_to_params_pointer;

  std::vector<std::string> _registered_kernel_names;
};

#endif //KERNELFACTORY_H
