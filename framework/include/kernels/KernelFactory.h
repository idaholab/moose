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
typedef Kernel * (*kernelBuildPtr)(const std::string & name, MooseSystem & moose_system, InputParameters parameters);

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
Kernel * buildKernel(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
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
  void registerKernel(const std::string & name)
  {
    if (_name_to_build_pointer.find(name) == _name_to_build_pointer.end())
    {
      _name_to_build_pointer[name] = &buildKernel<KernelType>;
      _name_to_params_pointer[name] = &validParams<KernelType>;
    }
    else
      mooseError("Kernel '" + name + "' already registered.");
  }

  Kernel *create(std::string kernel_name, const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  {
    return (*_name_to_build_pointer[kernel_name])(name, moose_system, parameters);
  }

  KernelNamesIterator registeredKernelsBegin();
  KernelNamesIterator registeredKernelsEnd();

  InputParameters getValidParams(const std::string & name);
  
private:
  KernelFactory();

  virtual ~KernelFactory();
  
  std::map<std::string, kernelBuildPtr> _name_to_build_pointer;
  std::map<std::string, kernelParamsPtr> _name_to_params_pointer;

  std::vector<std::string> _registered_kernel_names;
};

#endif //KERNELFACTORY_H
