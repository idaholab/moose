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

#include "ComputeNodalPPSThread.h"

#include "AuxiliarySystem.h"
#include "FEProblem.h"
#include "NodalPostprocessor.h"

// libmesh includes
#include "threads.h"

ComputeNodalPPSThread::ComputeNodalPPSThread(FEProblem & fe_problem,
                                             std::vector<PostprocessorWarehouse> & pps) :
    _fe_problem(fe_problem),
    _pps(pps)
{
}

// Splitting Constructor
ComputeNodalPPSThread::ComputeNodalPPSThread(ComputeNodalPPSThread & x, Threads::split /*split*/) :
    _fe_problem(x._fe_problem),
    _pps(x._pps)
{
}

void
ComputeNodalPPSThread::operator() (const ConstNodeRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  for (ConstNodeRange::const_iterator node_it = range.begin() ; node_it != range.end(); ++node_it)
  {
    const Node * node = *node_it;
    _fe_problem.reinitNode(node, _tid);

    // All Nodes
    for (std::vector<NodalPostprocessor *>::const_iterator nodal_postprocessor_it =
           _pps[_tid].nodalPostprocessors(Moose::ANY_BOUNDARY_ID).begin();
         nodal_postprocessor_it != _pps[_tid].nodalPostprocessors(Moose::ANY_BOUNDARY_ID).end();
         ++nodal_postprocessor_it)
    {
      (*nodal_postprocessor_it)->execute();
    }

    std::vector<BoundaryID> nodeset_ids = _fe_problem.mesh().getMesh().boundary_info->boundary_ids(node);

    for (std::vector<BoundaryID>::iterator it = nodeset_ids.begin(); it != nodeset_ids.end(); ++it)
    {
      for (std::vector<NodalPostprocessor *>::const_iterator nodal_postprocessor_it = _pps[_tid].nodalPostprocessors(*it).begin();
           nodal_postprocessor_it != _pps[_tid].nodalPostprocessors(*it).end();
           ++nodal_postprocessor_it)
      {
        (*nodal_postprocessor_it)->execute();
      }
    }
  }
}

void
ComputeNodalPPSThread::join(const ComputeNodalPPSThread & /*y*/)
{
}
