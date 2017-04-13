/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    displaced_node(i) = reference_node(i);
}

void
ResetDisplacedMeshThread::join(const ResetDisplacedMeshThread & /*y*/)
{
}
