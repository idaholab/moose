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
#include "Problem.h"
#include "FEProblem.h"

// libmesh includes
#include "threads.h"

ComputeNodalPPSThread::ComputeNodalPPSThread(Problem & problem,
                                             std::vector<PostprocessorWarehouse> & pps) :
    _problem(problem),
    _pps(pps)
{
}

// Splitting Constructor
ComputeNodalPPSThread::ComputeNodalPPSThread(ComputeNodalPPSThread & x, Threads::split /*split*/) :
    _problem(x._problem),
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

    _problem.reinitNode(node, _tid);

    for (std::vector<Postprocessor *>::const_iterator nodal_postprocessor_it = _pps[_tid].nodalPostprocessors().begin();
         nodal_postprocessor_it != _pps[_tid].nodalPostprocessors().end();
         ++nodal_postprocessor_it)
    {
      (*nodal_postprocessor_it)->execute();
    }
  }
}

void
ComputeNodalPPSThread::join(const ComputeNodalPPSThread & /*y*/)
{
}
