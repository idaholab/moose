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
#include "KokkosFESystem.h"

#include "MooseObject.h"

class FEProblemBase;

namespace Moose::Kokkos
{

class ReducerBase : public MeshHolder, public AssemblyHolder, public FESystemHolder
{
public:
  ReducerBase(const MooseObject * object);

  /**
   * Copy constructor for parallel dispatch
   */
  ReducerBase(const ReducerBase & reducer);

  /**
   * Kokkos function tag
   */
  struct ReducerLoop
  {
  };

  /**
   * Default methods to prevent compile errors even when these methods were not defined in the
   * derived class
   */
  ///@{
  template <typename Derived>
  KOKKOS_FUNCTION void reduce(Datum & /* datum */, Real * /* result */) const
  {
    ::Kokkos::abort("Default reduce() should never be called. Make sure you properly redefined "
                    "this method in your class without typos.");
  }
  template <typename Derived>
  KOKKOS_FUNCTION void join(Real * /* result */, const Real * /* source */) const
  {
    ::Kokkos::abort("Default join() should never be called. Make sure you properly redefined this "
                    "method in your class without typos.");
  }
  template <typename Derived>
  KOKKOS_FUNCTION void init(Real * /* result */) const
  {
    ::Kokkos::abort("Default init() should never be called. Make sure you properly redefined this "
                    "method in your class without typos.");
  }
  ///@}

  /**
   * Function used to check if users have overriden the hook method
   * @returns The function pointer of the default hook method
   */
  template <typename Derived>
  static auto defaultReduce()
  {
    return &ReducerBase::reduce<Derived>;
  }
  ///@}

protected:
  /**
   * Dispatch reduction operation
   */
  virtual void computeReducer();
  /**
   * Get the number of threads
   */
  virtual ThreadID numReducerThreads() const = 0;
  /**
   * Allocate reduction buffer
   */
  void allocateReductionBuffer(const unsigned int size)
  {
    ::Kokkos::realloc(_reduction_buffer, size);
  }

  /**
   * MOOSE object
   */
  const MooseObject * _reducer_object;
  /**
   * Kokkos functor dispatcher
   */
  std::unique_ptr<DispatcherBase> _reducer_dispatcher;
  /**
   * Reduction buffer
   */
  ::Kokkos::View<Real *, ::Kokkos::HostSpace> _reduction_buffer;
};

} // namespace Moose::Kokkos
