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

#include "Moose.h"
#include "ParallelUniqueId.h"
#include "MooseMesh.h"
#include "Problem.h"
#include "SystemBase.h"

/**
 * Base class for assembling-like calculations
 *
 */
template<typename RangeType>
class ThreadedElementLoop
{
public:
  ThreadedElementLoop(Problem & problem, SystemBase & system);

  ThreadedElementLoop(ThreadedElementLoop & x, Threads::split split);

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
  virtual void onBoundary(const Elem *elem, unsigned int side, short int bnd_id);

  /**
   * Called when doing internal edge assembling
   *
   * @param elem - Element we are on
   * @param side - local side number of the element 'elem'
   */
  virtual void onInternalSide(const Elem *elem, unsigned int side);

protected:
  SystemBase & _system;
  Problem & _problem;
  THREAD_ID _tid;
  unsigned int _subdomain;
};


template<typename RangeType>
ThreadedElementLoop<RangeType>::ThreadedElementLoop(Problem & problem, SystemBase & system) :
    _system(system),
    _problem(problem)
{
}

template<typename RangeType>
ThreadedElementLoop<RangeType>::ThreadedElementLoop(ThreadedElementLoop & x, Threads::split /*split*/) :
    _system(x._system),
    _problem(x._problem)
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

    onElement(elem);

    for (unsigned int side=0; side<elem->n_sides(); side++)
    {
      std::vector<short int> boundary_ids = _system.mesh().boundary_ids (elem, side);

      if (boundary_ids.size() > 0)
      {
        for (std::vector<short int>::iterator it = boundary_ids.begin(); it != boundary_ids.end(); ++it)
          onBoundary(elem, side, *it);
      }

      if (elem->neighbor(side) != NULL)
        onInternalSide(elem, side);
    } // sides
    postElement(elem);

    _subdomain = cur_subdomain;
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
ThreadedElementLoop<RangeType>::onBoundary(const Elem * /*elem*/, unsigned int /*side*/, short int /*bnd_id*/)
{
}

template<typename RangeType>
void
ThreadedElementLoop<RangeType>::onInternalSide(const Elem * /*elem*/, unsigned int /*side*/)
{
}

#endif //THREADEDELEMENTLOOP_H
