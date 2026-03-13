//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosNodalUserObject.h"

namespace Moose::Kokkos
{

InputParameters
NodalUserObject::validParams()
{
  InputParameters params = UserObject::validParams();
  params += NodalReducer::validParams();
  params += ::BoundaryRestrictable::validParams();
  params += ::TransientInterface::validParams();
  params += ::RandomInterface::validParams();

  return params;
}

NodalUserObject::NodalUserObject(const InputParameters & parameters)
  : UserObject(parameters),
    NodalReducer(this),
    ::CoupleableMooseVariableDependencyIntermediateInterface(this, true),
    ::TransientInterface(this),
    ::RandomInterface(parameters, _fe_problem, _tid, false)
{
}

NodalUserObject::NodalUserObject(const NodalUserObject & object)
  : UserObject(object),
    NodalReducer(object),
    ::CoupleableMooseVariableDependencyIntermediateInterface(object, {}),
    ::TransientInterface(object, {}),
    ::RandomInterface(object, {})
{
}

void
NodalUserObject::compute()
{
  if (DispatcherRegistry::hasDispatcher<ReducerLoop>(type()))
    reduce();
  else
    dispatch();
}

void
NodalUserObject::dispatch()
{
  Policy policy = _bnd ? Policy(0, numKokkosBoundaryNodes()) : Policy(0, numKokkosBlockNodes());

  if (!_dispatcher)
    _dispatcher = DispatcherRegistry::build<DefaultLoop>(this, type());

  _dispatcher->parallelFor(policy);
}

} // namespace Moose::Kokkos
