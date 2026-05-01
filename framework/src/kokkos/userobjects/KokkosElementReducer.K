//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosElementReducer.h"

namespace Moose::Kokkos
{

InputParameters
ElementReducer::validParams()
{
  InputParameters params = ::BlockRestrictable::validParams();
  return params;
}

ElementReducer::ElementReducer(const MooseObject * object)
  : ReducerBase(object), ::BlockRestrictable(object)
{
}

ElementReducer::ElementReducer(const ElementReducer & object)
  : ReducerBase(object), ::BlockRestrictable(object, {})
{
}

} // namespace Moose::Kokkos
