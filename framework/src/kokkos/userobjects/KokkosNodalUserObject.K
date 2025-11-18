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
  params += ::BlockRestrictable::validParams();
  params += ::BoundaryRestrictable::validParams();
  params += ::TransientInterface::validParams();
  params += ::RandomInterface::validParams();

  params.addParam<bool>("unique_node_execute",
                        false,
                        "When false (default), block restricted objects will have the "
                        "execute method called multiple times on a single node if the "
                        "node lies on a interface between two subdomains.");

  return params;
}

NodalUserObject::NodalUserObject(const InputParameters & parameters)
  : UserObject(parameters),
    ::BlockRestrictable(this),
    ::BoundaryRestrictable(this, blockIDs(), true),
    ::CoupleableMooseVariableDependencyIntermediateInterface(this, true),
    ::TransientInterface(this),
    ::RandomInterface(parameters, _fe_problem, _tid, false),
    _bnd(boundaryRestricted()),
    _unique_node_execute(getParam<bool>("unique_node_execute"))
{
}

NodalUserObject::NodalUserObject(const NodalUserObject & object)
  : UserObject(object),
    ::BlockRestrictable(object, {}),
    ::BoundaryRestrictable(object, {}),
    ::CoupleableMooseVariableDependencyIntermediateInterface(object, {}),
    ::TransientInterface(object, {}),
    ::RandomInterface(object, {}),
    _bnd(object._bnd),
    _unique_node_execute(object._unique_node_execute)
{
}

void
NodalUserObject::compute()
{
  Policy policy = _bnd ? Policy(0, numKokkosBoundaryNodes()) : Policy(0, numKokkosBlockNodes());

  if (!_dispatcher)
    _dispatcher = DispatcherRegistry::build<DefaultLoop>(this, type());

  _dispatcher->parallelFor(policy);
}

} // namespace Moose::Kokkos
