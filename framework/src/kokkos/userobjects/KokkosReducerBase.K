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

} // namespace Moose::Kokkos
