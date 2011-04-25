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
  PostprocessorIterator postprocessor_begin = _pps[_tid].elementPostprocessorsBegin(Moose::ANY_BLOCK_ID);
  PostprocessorIterator postprocessor_end = _pps[_tid].elementPostprocessorsEnd(Moose::ANY_BLOCK_ID);
  PostprocessorIterator postprocessor_it = postprocessor_begin;

  for (postprocessor_it=postprocessor_begin;postprocessor_it!=postprocessor_end;++postprocessor_it)
    (*postprocessor_it)->execute();

  postprocessor_begin = _pps[_tid].elementPostprocessorsBegin(subdomain);
  postprocessor_end = _pps[_tid].elementPostprocessorsEnd(subdomain);
  postprocessor_it = postprocessor_begin;

  for (postprocessor_it=postprocessor_begin;postprocessor_it!=postprocessor_end;++postprocessor_it)
    (*postprocessor_it)->execute();
}

void
ComputePostprocessorsThread::onBoundary(const Elem *elem, unsigned int side, short int bnd_id)
{
  PostprocessorIterator side_postprocessor_begin = _pps[_tid].sidePostprocessorsBegin(bnd_id);
  PostprocessorIterator side_postprocessor_end = _pps[_tid].sidePostprocessorsEnd(bnd_id);
  PostprocessorIterator side_postprocessor_it = side_postprocessor_begin;

  if (side_postprocessor_begin != side_postprocessor_end)
  {
    _problem.reinitElemFace(elem, side, bnd_id, _tid);
    _problem.reinitMaterialsFace(elem->subdomain_id(), side, _tid);

    for (; side_postprocessor_it!=side_postprocessor_end; ++side_postprocessor_it)
      (*side_postprocessor_it)->execute();
  }
}

void
ComputePostprocessorsThread::join(const ComputePostprocessorsThread & /*y*/)
{
}
