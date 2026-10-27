//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosSideUserObject.h"

namespace Moose::Kokkos
{

InputParameters
SideUserObject::validParams()
{
  InputParameters params = UserObject::validParams();
  params += SideReducer::validParams();
  params += ::MaterialPropertyInterface::validParams();
  params += ::TransientInterface::validParams();
  params += ::ElementIDInterface::validParams();
  return params;
}

SideUserObject::SideUserObject(const InputParameters & parameters)
  : UserObject(parameters),
    SideReducer(this),
    ::MaterialPropertyInterface(this, Moose::EMPTY_BLOCK_IDS, boundaryIDs()),
    ::CoupleableMooseVariableDependencyIntermediateInterface(this, false),
    ::TransientInterface(this),
    ::ElementIDInterface(this)
{
  // Kokkos user objects currently only consume variable information through finite element bases
  // and quadrature points
}

SideUserObject::SideUserObject(const SideUserObject & object)
  : UserObject(object),
    SideReducer(object),
    ::MaterialPropertyInterface(object, {}),
    ::CoupleableMooseVariableDependencyIntermediateInterface(object, {}),
    ::TransientInterface(object, {}),
    ::ElementIDInterface(object, {})
{
}

void
SideUserObject::compute()
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

} // namespace Moose::Kokkos
