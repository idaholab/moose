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

#ifndef THREADEDELEMENTLOOP_H
#define THREADEDELEMENTLOOP_H

#include "ParallelUniqueId.h"
#include "MooseMesh.h"
#include "FEProblem.h"
#include "SystemBase.h"

/**
 * Base class for assembling-like calculations
 *
 */
template<typename RangeType>
class ThreadedElementLoop
{
public:
  ThreadedElementLoop(FEProblem & feproblem, SystemBase & system);

  ThreadedElementLoop(ThreadedElementLoop & x, Threads::split split);

  virtual ~ThreadedElementLoop();

  void operator() (const RangeType & range);

  /**
   * Called before the element range loop
   */
  virtual void pre();

  /**
   * Called after the element range loop
   */
  virtual void post();

  /**
   * Assembly of the element (not including surface assembly)
   *
   * @param elem - active element
   */
  virtual void onElement(const Elem *elem);

  /**
   * Called after the element assembly is done (including surface assembling)
   *
   * @param elem - active element
   */
  virtual void postElement(const Elem *elem);

  /**
   * Called when doing boundary assembling
   *
   * @param bnd_id - ID of the boundary we are at
   */
  virtual void onBoundary(const Elem *elem, unsigned int side, BoundaryID bnd_id);

  /**
   * Called when doing internal edge assembling
   *
   * @param elem - Element we are on
   * @param side - local side number of the element 'elem'
   */
  virtual void onInternalSide(const Elem *elem, unsigned int side);

  /**
   * Called every time the current subdoain changes (ie the subdomain of _this_ element
   * is not the same as the subdomain of the last element).  Beware of overusing this!
   * You might think that you can do some expensive stuff in here and get away with it...
   * but there are applications that have TONS of subdomains....
   */
  virtual void subdomainChanged();

protected:
  SystemBase & _system;
  FEProblem & _fe_problem;
  THREAD_ID _tid;

  /// The subdomain for the current element
  unsigned int _subdomain;

  /// The subdomain for the last element
  unsigned int _old_subdomain;
};


template<typename RangeType>
ThreadedElementLoop<RangeType>::ThreadedElementLoop(FEProblem & fe_problem, SystemBase & system) :
    _system(system),
    _fe_problem(fe_problem)
{
}

template<typename RangeType>
ThreadedElementLoop<RangeType>::ThreadedElementLoop(ThreadedElementLoop & x, Threads::split /*split*/) :
    _system(x._system),
    _fe_problem(x._fe_problem)
{
}

template<typename RangeType>
ThreadedElementLoop<RangeType>::~ThreadedElementLoop()
{
}

template<typename RangeType>
void
ThreadedElementLoop<RangeType>::operator () (const RangeType & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  pre();

  _subdomain = std::numeric_limits<unsigned int>::max();
  typename RangeType::const_iterator el = range.begin();
  for (el = range.begin() ; el != range.end(); ++el)
  {
    const Elem* elem = *el;
    unsigned int cur_subdomain = elem->subdomain_id();

    _old_subdomain = _subdomain;
    _subdomain = cur_subdomain;

    if(_subdomain != _old_subdomain)
      subdomainChanged();

    onElement(elem);

    for (unsigned int side=0; side<elem->n_sides(); side++)
    {
      std::vector<BoundaryID> boundary_ids = _system.mesh().boundary_ids (elem, side);

      if (boundary_ids.size() > 0)
        for (std::vector<BoundaryID>::iterator it = boundary_ids.begin(); it != boundary_ids.end(); ++it)
          onBoundary(elem, side, *it);

      if (elem->neighbor(side) != NULL)
        onInternalSide(elem, side);
    } // sides
    postElement(elem);

  } // range

  post();
}

template<typename RangeType>
void
ThreadedElementLoop<RangeType>::pre()
{

}

template<typename RangeType>
void
ThreadedElementLoop<RangeType>::post()
{

}

template<typename RangeType>
void
ThreadedElementLoop<RangeType>::onElement(const Elem * /*elem*/)
{
}

template<typename RangeType>
void
ThreadedElementLoop<RangeType>::postElement(const Elem * /*elem*/)
{
}

template<typename RangeType>
void
ThreadedElementLoop<RangeType>::onBoundary(const Elem * /*elem*/, unsigned int /*side*/, BoundaryID /*bnd_id*/)
{
}

template<typename RangeType>
void
ThreadedElementLoop<RangeType>::onInternalSide(const Elem * /*elem*/, unsigned int /*side*/)
{
}

template<typename RangeType>
void
ThreadedElementLoop<RangeType>::subdomainChanged()
{
}

#endif //THREADEDELEMENTLOOP_H
