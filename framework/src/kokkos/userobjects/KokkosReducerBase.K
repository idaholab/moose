//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosReducerBase.h"

#include "FEProblemBase.h"

namespace Moose::Kokkos
{

ReducerBase::ReducerBase(const MooseObject * object)
  : MeshHolder(*object->getMooseApp().feProblem().mesh().getKokkosMesh()),
    AssemblyHolder(object->getMooseApp().feProblem().kokkosAssembly()),
    SystemHolder(object->getMooseApp().feProblem().getKokkosSystems()),
    _reducer_object(object)
{
}

ReducerBase::ReducerBase(const ReducerBase & reducer)
  : MeshHolder(reducer),
    AssemblyHolder(reducer),
    SystemHolder(reducer),
    _reducer_object(reducer._reducer_object)
{
}

void
ReducerBase::computeReducer()
{
  if (!_reduction_buffer.is_allocated())
    _reducer_object->mooseError("Reduction buffer was not allocated.");

  Policy policy(0, numReducerThreads());

  if (!_reducer_dispatcher)
    _reducer_dispatcher =
        DispatcherRegistry::build<ReducerLoop>(_reducer_object, _reducer_object->type());

  _reducer_dispatcher->parallelReduce(policy, _reduction_buffer);
}

} // namespace Moose::Kokkos
