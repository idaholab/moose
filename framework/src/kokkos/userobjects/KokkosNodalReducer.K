//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosNodalReducer.h"

namespace Moose::Kokkos
{

InputParameters
NodalReducer::validParams()
{
  InputParameters params = ::BlockRestrictable::validParams();
  params += ::BoundaryRestrictable::validParams();

  params.addParam<bool>("unique_node_execute",
                        false,
                        "When false (default), block restricted objects will have the "
                        "execute method called multiple times on a single node if the "
                        "node lies on a interface between two subdomains.");

  return params;
}

NodalReducer::NodalReducer(const MooseObject * object)
  : ReducerBase(object),
    ::BlockRestrictable(object),
    ::BoundaryRestrictable(object, blockIDs(), true),
    _bnd(boundaryRestricted()),
    _unique_node_execute(object->getParam<bool>("unique_node_execute"))
{
}

NodalReducer::NodalReducer(const NodalReducer & object)
  : ReducerBase(object),
    ::BlockRestrictable(object, {}),
    ::BoundaryRestrictable(object, {}),
    _bnd(object._bnd),
    _unique_node_execute(object._unique_node_execute)
{
}

} // namespace Moose::Kokkos
