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
  virtual void computeReducer() = 0;

  /**
   * Kokkos function tag
   */
  struct ReducerLoop
  {
  };

  /**
   * Shim for hook method that can be leveraged to implement static polymorphism
   */
  template <typename Derived>
  KOKKOS_FUNCTION void reduceShim(const Derived & reducer, Datum & datum, Real * result) const
  {
    reducer.reduce(datum, result);
  }

  /**
   * Default methods to prevent compile errors even when these methods were not defined in the
   * derived class
   */
  ///@{
  KOKKOS_FUNCTION void reduce(Datum & /* datum */, Real * /* result */) const
  {
    ::Kokkos::abort("Default reduce() should never be called. Make sure you properly redefined "
                    "this method in your class without typos.");
  }
  KOKKOS_FUNCTION void join(ReducerLoop, Real * /* result */, const Real * /* source */) const
  {
    ::Kokkos::abort("Default join() should never be called. Make sure you properly redefined this "
                    "method in your class without typos.");
  }
  KOKKOS_FUNCTION void init(ReducerLoop, Real * /* result */) const
  {
    ::Kokkos::abort("Default init() should never be called. Make sure you properly redefined this "
                    "method in your class without typos.");
  }
  ///@}

  /**
   * Functions used to check if users have overriden the hook methods, whose calculations can be
   * skipped when not overriden
   * @returns The function pointer of the default hook method
   */
  ///@{
  template <typename Derived>
  static auto defaultReduceShim()
  {
    return &ReducerBase::reduceShim<Derived>;
  }
  static auto defaultReduce() { return &ReducerBase::reduce; }
  ///@}

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
