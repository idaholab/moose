//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosSideUserObject.h"

namespace Moose
{
namespace Kokkos
{

InputParameters
SideUserObject::validParams()
{
  InputParameters params = UserObject::validParams();
  params += ::BoundaryRestrictableRequired::validParams();
  params += ::MaterialPropertyInterface::validParams();
  params += ::TransientInterface::validParams();
  params += ::ElementIDInterface::validParams();
  return params;
}

SideUserObject::SideUserObject(const InputParameters & parameters)
  : UserObject(parameters),
    ::BoundaryRestrictableRequired(this, false),
    ::MaterialPropertyInterface(this, Moose::EMPTY_BLOCK_IDS, boundaryIDs()),
    ::CoupleableMooseVariableDependencyIntermediateInterface(this, false),
    ::TransientInterface(this),
    ::ElementIDInterface(this)
{
}

SideUserObject::SideUserObject(const SideUserObject & object)
  : UserObject(object),
    ::BoundaryRestrictableRequired(object, {}),
    ::MaterialPropertyInterface(object, {}),
    ::CoupleableMooseVariableDependencyIntermediateInterface(object, {}),
    ::TransientInterface(object, {}),
    ::ElementIDInterface(object, {})
{
}

void
SideUserObject::compute()
{
  Policy policy(0, numKokkosBoundarySides());

  if (!_dispatcher)
    _dispatcher = DispatcherRegistry::build<DefaultLoop>(this, type());

  _dispatcher->parallelFor(policy);
}

} // namespace Kokkos
} // namespace Moose
