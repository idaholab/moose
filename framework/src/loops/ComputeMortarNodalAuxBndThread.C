//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
    const TheWarehouse::Query & query,
    const BoundaryID bnd_id,
    const std::size_t object_container_index)
  : ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>(fe_problem),
    _aux_sys(fe_problem.getAuxiliarySystem()),
    _query(query),
    _query_boundary(_query),
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
    _query(x._query),
    _query_boundary(x._query_boundary),
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
    std::vector<AuxKernelType *> kernels;
    _query_boundary.queryInto(kernels, _tid, std::make_tuple(_bnd_id, /*must_be_restricted=*/true));
    mooseAssert(_object_container_index < kernels.size(),
                "The mortar nodal aux kernel index is out of range for this boundary.");
    auto * kernel = kernels[_object_container_index];
    mooseAssert(dynamic_cast<MortarNodalAuxKernel *>(kernel),
                "This should be a mortar nodal aux kernel");
    _fe_problem.reinitNodeFace(node, _bnd_id, _tid);
    kernel->compute();
    // This is the same conditional check that the aux kernel performs internally before calling
    // computeValue and _var.setNodalValue. We don't want to attempt to insert into the solution if
    // we don't actually have any dofs on this node
    if (kernel->variable().isNodalDefined())
      kernel->variable().insert(_aux_sys.solution());
  }
}

template <typename AuxKernelType>
void
ComputeMortarNodalAuxBndThread<AuxKernelType>::join(const ComputeMortarNodalAuxBndThread & /*y*/)
{
}

template class ComputeMortarNodalAuxBndThread<AuxKernel>;
