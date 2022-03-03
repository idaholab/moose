//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ResetDisplacedMeshThread.h"
#include "DisplacedProblem.h"
#include "MooseMesh.h"

#include "SubProblem.h"

ResetDisplacedMeshThread::ResetDisplacedMeshThread(FEProblemBase & fe_problem,
                                                   DisplacedProblem & displaced_problem)
  : ThreadedNodeLoop<NodeRange, NodeRange::const_iterator>(fe_problem),
    _displaced_problem(displaced_problem),
    _ref_mesh(_displaced_problem.refMesh())
{
}

ResetDisplacedMeshThread::ResetDisplacedMeshThread(ResetDisplacedMeshThread & x,
                                                   Threads::split split)
  : ThreadedNodeLoop<NodeRange, NodeRange::const_iterator>(x, split),
    _displaced_problem(x._displaced_problem),
    _ref_mesh(x._ref_mesh)
{
}

void
ResetDisplacedMeshThread::onNode(NodeRange::const_iterator & nd)
{
  Node & displaced_node = **nd;

  // Get the same node from the reference mesh.
  Node & reference_node = _ref_mesh.nodeRef(displaced_node.id());

  // Undisplace the node
  for (const auto i : make_range(Moose::dim))
    displaced_node(i) = reference_node(i);
}

void
ResetDisplacedMeshThread::join(const ResetDisplacedMeshThread & /*y*/)
{
}
