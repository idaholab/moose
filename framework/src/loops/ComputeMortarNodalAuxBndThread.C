//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "libmesh/threads.h"

// MOOSE includes
#include "ComputeMortarNodalAuxBndThread.h"
#include "AuxiliarySystem.h"
#include "FEProblem.h"
#include "MortarNodalAuxKernel.h"

template <typename AuxKernelType>
ComputeMortarNodalAuxBndThread<AuxKernelType>::ComputeMortarNodalAuxBndThread(
    FEProblemBase & fe_problem,
    const MooseObjectWarehouse<AuxKernelType> & storage,
    const BoundaryID bnd_id,
    const std::size_t object_container_index)
  : ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>(fe_problem),
    _aux_sys(fe_problem.getAuxiliarySystem()),
    _storage(storage),
    _bnd_id(bnd_id),
    _object_container_index(object_container_index)
{
}

// Splitting Constructor
template <typename AuxKernelType>
ComputeMortarNodalAuxBndThread<AuxKernelType>::ComputeMortarNodalAuxBndThread(
    ComputeMortarNodalAuxBndThread & x, Threads::split split)
  : ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>(x, split),
    _aux_sys(x._aux_sys),
    _storage(x._storage),
    _bnd_id(x._bnd_id),
    _object_container_index(x._object_container_index)
{
}

template <typename AuxKernelType>
void
ComputeMortarNodalAuxBndThread<AuxKernelType>::onNode(ConstBndNodeRange::const_iterator & node_it)
{
  const BndNode * bnode = *node_it;

  if (bnode->_bnd_id != _bnd_id)
    return;

  Node * node = bnode->_node;

  if (node->processor_id() == _fe_problem.processor_id())
  {
    const auto & kernel = _storage.getActiveBoundaryObjects(_bnd_id, _tid)[_object_container_index];
    mooseAssert(dynamic_cast<MortarNodalAuxKernel *>(kernel.get()),
                "This should be amortar nodal aux kernel");
    _fe_problem.reinitNodeFace(node, _bnd_id, _tid);
    kernel->compute();
    // update the solution vector
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    kernel->variable().insert(_aux_sys.solution());
  }
}

template <typename AuxKernelType>
void
ComputeMortarNodalAuxBndThread<AuxKernelType>::join(const ComputeMortarNodalAuxBndThread & /*y*/)
{
}

template class ComputeMortarNodalAuxBndThread<AuxKernel>;
