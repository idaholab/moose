//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosElementUserObject.h"

namespace Moose::Kokkos
{

InputParameters
ElementUserObject::validParams()
{
  InputParameters params = UserObject::validParams();
  params += ElementReducer::validParams();
  params += ::MaterialPropertyInterface::validParams();
  params += ::TransientInterface::validParams();
  params += ::RandomInterface::validParams();
  return params;
}

ElementUserObject::ElementUserObject(const InputParameters & parameters)
  : UserObject(parameters),
    ElementReducer(this),
    ::MaterialPropertyInterface(this, blockIDs(), Moose::EMPTY_BOUNDARY_IDS),
    ::CoupleableMooseVariableDependencyIntermediateInterface(this, false),
    ::TransientInterface(this),
    ::RandomInterface(parameters, _fe_problem, _tid, false),
    ::ElementIDInterface(this)
{
}

ElementUserObject::ElementUserObject(const ElementUserObject & object)
  : UserObject(object),
    ElementReducer(object),
    ::MaterialPropertyInterface(object, {}),
    ::CoupleableMooseVariableDependencyIntermediateInterface(object, {}),
    ::TransientInterface(object, {}),
    ::RandomInterface(object, {}),
    ::ElementIDInterface(object, {})
{
}

void
ElementUserObject::compute()
{
  if (DispatcherRegistry::hasUserMethod<DefaultLoop>(type()) ==
      DispatcherRegistry::hasUserMethod<ReducerLoop>(type()))
    mooseError("Cannot determine whether '",
               name(),
               "' is a regular user object or a reducer object. Either none or both of the "
               "execute() and reduce() methods were specified. Check if they were defined properly "
               "and override compute() if you want to manually choose the behavior.");

  if (DispatcherRegistry::hasUserMethod<ReducerLoop>(type()))
    computeReducer();
  else
    computeUserObject();
}

void
ElementUserObject::computeUserObject()
{
  Policy policy(0, numKokkosBlockElements());

  if (!_dispatcher)
    _dispatcher = DispatcherRegistry::build<DefaultLoop>(this, type());

  _dispatcher->parallelFor(policy);
}

} // namespace Moose::Kokkos
