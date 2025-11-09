//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosNodalReducer.h"

namespace Moose
{
namespace Kokkos
{

InputParameters
NodalReducer::validParams()
{
  InputParameters params = NodalUserObject::validParams();
  return params;
}

NodalReducer::NodalReducer(const InputParameters & parameters) : NodalUserObject(parameters) {}

void
NodalReducer::compute()
{
  if (!_reduction_buffer.is_allocated())
    mooseError("Reduction buffer was not allocated.");

  Policy policy = _bnd ? Policy(0, numKokkosBoundaryNodes()) : Policy(0, numKokkosBlockNodes());

  if (!_dispatcher)
    _dispatcher = DispatcherRegistry::build<DefaultLoop>(this, type());

  _dispatcher->parallelReduce(policy, _reduction_buffer);
}

} // namespace Kokkos
} // namespace Moose
