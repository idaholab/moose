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
ComputePostprocessorsThread::pre()
{
  //Initialize side and element post processors

  std::set<unsigned int>::iterator block_begin = _pps[_tid]._block_ids_with_postprocessors.begin();
  std::set<unsigned int>::iterator block_end = _pps[_tid]._block_ids_with_postprocessors.end();
  std::set<unsigned int>::iterator block_it = block_begin;

  for (block_it=block_begin;block_it!=block_end;++block_it)
  {
    unsigned int block_id = *block_it;

    PostprocessorIterator postprocessor_begin = _pps[_tid].elementPostprocessorsBegin(block_id);
    PostprocessorIterator postprocessor_end = _pps[_tid].elementPostprocessorsEnd(block_id);
    PostprocessorIterator postprocessor_it = postprocessor_begin;

    for (postprocessor_it=postprocessor_begin;postprocessor_it!=postprocessor_end;++postprocessor_it)
      (*postprocessor_it)->initialize();
  }

  std::set<unsigned int>::iterator boundary_begin = _pps[_tid]._boundary_ids_with_postprocessors.begin();
  std::set<unsigned int>::iterator boundary_end = _pps[_tid]._boundary_ids_with_postprocessors.end();
  std::set<unsigned int>::iterator boundary_it = boundary_begin;

  for (boundary_it=boundary_begin;boundary_it!=boundary_end;++boundary_it)
  {
    //note: for threaded applications where the elements get broken up it
    //may be more efficient to initialize these on demand inside the loop
    PostprocessorIterator side_postprocessor_begin = _pps[_tid].sidePostprocessorsBegin(*boundary_it);
    PostprocessorIterator side_postprocessor_end = _pps[_tid].sidePostprocessorsEnd(*boundary_it);
    PostprocessorIterator side_postprocessor_it = side_postprocessor_begin;

    for (side_postprocessor_it=side_postprocessor_begin;side_postprocessor_it!=side_postprocessor_end;++side_postprocessor_it)
      (*side_postprocessor_it)->initialize();
  }
}

void
ComputePostprocessorsThread::preElement(const Elem * elem)
{
  _problem.prepare(elem, _tid);
  _problem.reinitElem(elem, _tid);
  _problem.reinitMaterials(elem->subdomain_id(), _tid);
}

void
ComputePostprocessorsThread::onElement(const Elem *elem)
{
  unsigned int subdomain = elem->subdomain_id();

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
