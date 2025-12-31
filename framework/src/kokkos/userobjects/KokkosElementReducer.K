//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosElementReducer.h"

namespace Moose
{
namespace Kokkos
{

InputParameters
ElementReducer::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  return params;
}

ElementReducer::ElementReducer(const InputParameters & parameters) : ElementUserObject(parameters)
{
}

void
ElementReducer::compute()
{
  if (!_reduction_buffer.is_allocated())
    mooseError("Reduction buffer was not allocated.");

  Policy policy(0, numKokkosBlockElements());

  if (!_dispatcher)
    _dispatcher = DispatcherRegistry::build<DefaultLoop>(this, type());

  _dispatcher->parallelReduce(policy, _reduction_buffer);
}

} // namespace Kokkos
} // namespace Moose
