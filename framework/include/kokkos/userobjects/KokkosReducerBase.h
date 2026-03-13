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
#include "KokkosDispatcher.h"

#include "MooseObject.h"

class FEProblemBase;

namespace Moose::Kokkos
{

class ReducerBase : public MeshHolder, public AssemblyHolder, public SystemHolder
{
public:
  ReducerBase(const MooseObject * object);

  /**
   * Copy constructor for parallel dispatch
   */
  ReducerBase(const ReducerBase & reducer);

  /**
   * Dispatch reduction operation
   */
  virtual void reduce() = 0;

  /**
   * Shim for hook method that can be leveraged to implement static polymorphism
   */
  template <typename Derived>
  KOKKOS_FUNCTION void executeShim(const Derived & reducer, Datum & datum, Real * result) const
  {
    reducer.execute(datum, result);
  }

  /**
   * Kokkos function tag
   */
  struct ReducerLoop
  {
  };

protected:
  /**
   * MOOSE object
   */
  const MooseObject * _reducer_object;
  /**
   * Kokkos functor dispatcher
   */
  std::unique_ptr<DispatcherBase> _dispatcher;
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
