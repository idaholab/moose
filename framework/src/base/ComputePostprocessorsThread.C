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

#include "ComputePostprocessorsThread.h"

#include "Problem.h"
#include "SystemBase.h"

#include "ElementPostprocessor.h"
#include "SidePostprocessor.h"

ComputePostprocessorsThread::ComputePostprocessorsThread(Problem & problem, SystemBase & sys, const NumericVector<Number>& in_soln, std::vector<PostprocessorWarehouse> & pps) :
    ThreadedElementLoop<ConstElemRange>(problem, sys),
    _soln(in_soln),
    _pps(pps)
{}

// Splitting Constructor
ComputePostprocessorsThread::ComputePostprocessorsThread(ComputePostprocessorsThread & x, Threads::split) :
    ThreadedElementLoop<ConstElemRange>(x._problem, x._system),
    _soln(x._soln),
    _pps(x._pps)
{}

void
ComputePostprocessorsThread::onElement(const Elem * elem)
{
  unsigned int subdomain = elem->subdomain_id();

  _problem.prepare(elem, _tid);
  _problem.reinitElem(elem, _tid);
  _problem.reinitMaterials(subdomain, _tid);

  //Global Postprocessors
  for (std::vector<ElementPostprocessor *>::const_iterator postprocessor_it = _pps[_tid].elementPostprocessors(Moose::ANY_BLOCK_ID).begin();
      postprocessor_it != _pps[_tid].elementPostprocessors(Moose::ANY_BLOCK_ID).end();
      ++postprocessor_it)
    (*postprocessor_it)->execute();

  for (std::vector<ElementPostprocessor *>::const_iterator postprocessor_it = _pps[_tid].elementPostprocessors(subdomain).begin();
      postprocessor_it != _pps[_tid].elementPostprocessors(subdomain).end();
      ++postprocessor_it)
    (*postprocessor_it)->execute();
}

void
ComputePostprocessorsThread::onBoundary(const Elem *elem, unsigned int side, BoundaryID bnd_id)
{
  if (_pps[_tid].sidePostprocessors(bnd_id).size() > 0)
  {
    _problem.reinitElemFace(elem, side, bnd_id, _tid);
    _problem.reinitMaterialsFace(elem->subdomain_id(), side, _tid);

    for (std::vector<SidePostprocessor *>::const_iterator side_postprocessor_it = _pps[_tid].sidePostprocessors(bnd_id).begin();
        side_postprocessor_it != _pps[_tid].sidePostprocessors(bnd_id).end();
        ++side_postprocessor_it)
      (*side_postprocessor_it)->execute();
  }
}

void
ComputePostprocessorsThread::join(const ComputePostprocessorsThread & /*y*/)
{
}
