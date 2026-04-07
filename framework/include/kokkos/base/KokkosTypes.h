//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosThread.h"
#include "KokkosScalar.h"
#include "KokkosJaggedArray.h"

#include "MooseError.h"
#include "MooseUtils.h"

#include "libmesh/tensor_tools.h"

// Real3 and Real33 now live in libMesh::Kokkos (libmesh/kokkos/scalar_types.h).
// Pull them into Moose::Kokkos for backward compatibility with existing MOOSE code.
#include "libmesh/kokkos/scalar_types.h"

namespace Moose::Kokkos
{

using libMesh::Kokkos::Real;
using libMesh::Kokkos::Real3;
using libMesh::Kokkos::Real33;
#ifdef MOOSE_KOKKOS_SCOPE
using libMesh::Kokkos::operator*;
using libMesh::Kokkos::operator+;
using libMesh::Kokkos::operator-;
#endif

template <typename T1, typename T2>
struct Pair
{
  T1 first;
  T2 second;

  template <typename T3, typename T4>
  auto & operator=(const std::pair<T3, T4> pair)
  {
    first = pair.first;
    second = pair.second;

    return *this;
  }
};

template <typename T1, typename T2>
bool
operator<(const Pair<T1, T2> & left, const Pair<T1, T2> & right)
{
  return std::make_pair(left.first, left.second) < std::make_pair(right.first, right.second);
}

} // namespace Moose::Kokkos
