//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosTypes.h"
#include "DataIO.h"

#include <iterator>

// Serialization of the Kokkos tensor types, which a material property needs in order to be stateful:
// MaterialData instantiates the property serialization functions only for stateful properties, so a
// value type without these is usable as a current-state property and rejected at compile time as an
// old-state one.
//
// These live in Moose::Kokkos so that argument-dependent lookup finds them ahead of the generic
// dataStore and dataLoad templates, whose static_assert reports an unserializable type.

namespace Moose::Kokkos
{

inline void
dataStore(std::ostream & stream, Real6 & tensor, void * context)
{
  for (std::size_t i = 0; i < std::size(tensor.a); ++i)
    ::dataStore(stream, tensor.a[i], context);
}

inline void
dataLoad(std::istream & stream, Real6 & tensor, void * context)
{
  for (std::size_t i = 0; i < std::size(tensor.a); ++i)
    ::dataLoad(stream, tensor.a[i], context);
}

} // namespace Moose::Kokkos
