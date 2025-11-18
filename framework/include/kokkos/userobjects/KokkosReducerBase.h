//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosHeader.h"
#include "KokkosDatum.h"

namespace Moose::Kokkos
{

class ReducerBase
{
public:
  /**
   * Shim for hook method that can be leveraged to implement static polymorphism
   */
  template <typename Derived>
  KOKKOS_FUNCTION void executeShim(const Derived & reducer, Datum & datum, Real * result) const
  {
    reducer.execute(datum, result);
  }

protected:
  /**
   * Reduction buffer
   */
  ::Kokkos::View<Real *, ::Kokkos::HostSpace> _reduction_buffer;
  /**
   * Allocate reduction buffer
   */
  void allocateReductionBuffer(const unsigned int size)
  {
    ::Kokkos::realloc(_reduction_buffer, size);
  }
};

} // namespace Moose::Kokkos
