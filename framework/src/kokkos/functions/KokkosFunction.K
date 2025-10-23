//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosFunction.h"

namespace Moose
{
namespace Kokkos
{

InputParameters
FunctionBase::validParams()
{
  InputParameters params = Moose::FunctionBase::validParams();
  params.registerBase("KokkosFunction");

  return params;
}

FunctionBase::FunctionBase(const InputParameters & parameters) : Moose::FunctionBase(parameters) {}

FunctionBase::FunctionBase(const FunctionBase & object) : Moose::FunctionBase(object, {}) {}

Function::Function(std::shared_ptr<FunctionWrapperHostBase> wrapper) : _wrapper_host(wrapper)
{
  // Allocate device wrapper
  _wrapper_device = _wrapper_host->allocate();
}

Function::Function(const Function & function)
  : _wrapper_host(function._wrapper_host), _wrapper_device(function._wrapper_device)
{
  // Copy function to device
  _wrapper_host->copyFunction();
}

Function::~Function()
{
  // Free device wrapper
  if (_wrapper_host.use_count() == 1)
    ::Kokkos::kokkos_free<ExecSpace::memory_space>(_wrapper_device);

  // Free host copy of function
  _wrapper_host->freeFunction();
}

} // namespace Kokkos
} // namespace Moose
