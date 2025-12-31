//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosSideReducer.h"

namespace Moose
{
namespace Kokkos
{

InputParameters
SideReducer::validParams()
{
  InputParameters params = SideUserObject::validParams();
  return params;
}

SideReducer::SideReducer(const InputParameters & parameters) : SideUserObject(parameters) {}

void
SideReducer::compute()
{
  if (!_reduction_buffer.is_allocated())
    mooseError("Reduction buffer was not allocated.");

  Policy policy(0, numKokkosBoundarySides());

  if (!_dispatcher)
    _dispatcher = DispatcherRegistry::build<DefaultLoop>(this, type());

  _dispatcher->parallelReduce(policy, _reduction_buffer);
}

} // namespace Kokkos
} // namespace Moose
