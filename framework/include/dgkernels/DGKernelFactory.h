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

#ifndef DGKERNELFACTORY_H
#define DGKERNELFACTORY_H

#include "DGKernel.h"

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
typedef DGKernel * (*dgKernelBuildPtr)(const std::string & name, MooseSystem & moose_system, InputParameters parameters);

/**
 * Typedef to hide implementation details
 */
typedef std::vector<std::string>::iterator DGKernelNamesIterator;

/**
 * Typedef to make things easier.
 */
typedef InputParameters (*dgKernelParamsPtr)();

/**
 * Templated build function used for generating function pointers to build classes on demand.
 */
template<typename DGKernelType>
DGKernel * buildDGKernel(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
{
  return new DGKernelType(name, moose_system, parameters);
}

/**
 * Responsible for building Kernels on demand and storing them for retrieval
 */
class DGKernelFactory
{
public:
  static DGKernelFactory * instance();

  template<typename DGKernelType>
  void registerDGKernel(const std::string & name)
  {
    _name_to_build_pointer[name]=&buildDGKernel<DGKernelType>;
    _name_to_params_pointer[name]=&validParams<DGKernelType>;
  }

  DGKernel *create(std::string kernel_name, const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  {
    return (*_name_to_build_pointer[kernel_name])(name, moose_system, parameters);
  }

  DGKernelNamesIterator registeredDGKernelsBegin();
  DGKernelNamesIterator registeredDGKernelsEnd();

  InputParameters getValidParams(const std::string & name);
  
private:
  DGKernelFactory();

  virtual ~DGKernelFactory();
  
  std::map<std::string, dgKernelBuildPtr> _name_to_build_pointer;
  std::map<std::string, dgKernelParamsPtr> _name_to_params_pointer;

  std::vector<std::string> _registered_dgkernel_names;
};

#endif //DGKERNELFACTORY_H
