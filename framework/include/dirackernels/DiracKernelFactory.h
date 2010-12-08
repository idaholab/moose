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

#ifndef DIRACKERNELFACTORY_H
#define DIRACKERNELFACTORY_H

#include "DiracKernel.h"

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
typedef DiracKernel * (*dirac_kernelBuildPtr)(const std::string & name, InputParameters parameters);

/**
 * Typedef to hide implementation details
 */
typedef std::vector<std::string>::iterator DiracKernelNamesIterator;

/**
 * Typedef to make things easier.
 */
typedef InputParameters (*dirac_kernelParamsPtr)();

/**
 * Templated build function used for generating function pointers to build classes on demand.
 */
template<typename DiracKernelType>
DiracKernel * buildDiracKernel(const std::string & name, InputParameters parameters)
{
  return new DiracKernelType(name, parameters);
}

/**
 * Responsible for building DiracKernels on demand and storing them for retrieval
 */
class DiracKernelFactory
{
public:
  static DiracKernelFactory * instance();

  template<typename DiracKernelType> 
  void registerDiracKernel(const std::string & name)
  {
    if (_name_to_build_pointer.find(name) == _name_to_build_pointer.end())
    {
      _name_to_build_pointer[name] = &buildDiracKernel<DiracKernelType>;
      _name_to_params_pointer[name] = &validParams<DiracKernelType>;
    }
    else
      mooseError("DiracKernel '" + name + "' already registered.");
  }

  DiracKernel *create(std::string dirac_kernel_name, const std::string & name, InputParameters parameters)
  {
    return (*_name_to_build_pointer[dirac_kernel_name])(name, parameters);
  }

  DiracKernelNamesIterator registeredDiracKernelsBegin();
  DiracKernelNamesIterator registeredDiracKernelsEnd();

  InputParameters getValidParams(const std::string & name);
  
private:
  DiracKernelFactory();

  virtual ~DiracKernelFactory();
  
  std::map<std::string, dirac_kernelBuildPtr> _name_to_build_pointer;
  std::map<std::string, dirac_kernelParamsPtr> _name_to_params_pointer;

  std::vector<std::string> _registered_dirac_kernel_names;
};

#endif //DIRACKERNELFACTORY_H
