#ifndef THREADEDELEMENTLOOP_H_
#define THREADEDELEMENTLOOP_H_

#include "Moose.h"
#include "ParallelUniqueId.h"
#include "Mesh.h"

namespace Moose {

/**
 * Base class for assembling-like calculations
 *
 */
template<typename RangeType>
class ThreadedElementLoop
{
public:
  ThreadedElementLoop(Problem & problem, System & system);

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
   * Called before an element assembly
   *
   * @param elem - active element
   */
  virtual void preElement(const Elem *elem);

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
   * Called when subdomain has changed
   *
   * @param subdomain - ID of the new subdomain
   */
  virtual void onDomainChanged(short int subdomain);

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
  System & _system;
  Problem & _problem;
  THREAD_ID _tid;
};


template<typename RangeType>
ThreadedElementLoop<RangeType>::ThreadedElementLoop(Problem & problem, System & system) :
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

  unsigned int subdomain = std::numeric_limits<unsigned int>::max();
  typename RangeType::const_iterator el = range.begin();
  for (el = range.begin() ; el != range.end(); ++el)
  {
    const Elem* elem = *el;
    unsigned int cur_subdomain = elem->subdomain_id();

    preElement(elem);

    if (cur_subdomain != subdomain)
    {
      subdomain = cur_subdomain;
      onDomainChanged(subdomain);
    }

    onElement(elem);

//    postElement(elem);

    for (unsigned int side=0; side<elem->n_sides(); side++)
    {
      std::vector<short int> boundary_ids = _problem.mesh().boundary_ids (elem, side);

      if (boundary_ids.size() > 0)
      {
        for (std::vector<short int>::iterator it = boundary_ids.begin(); it != boundary_ids.end(); ++it)
          onBoundary(elem, side, *it);
      }

      if (elem->neighbor(side) != NULL)
        onInternalSide(elem, side);
    } // sides
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
ThreadedElementLoop<RangeType>::preElement(const Elem * /*elem*/)
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
ThreadedElementLoop<RangeType>::onDomainChanged(short int /*subdomain*/)
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

} // namespace

#endif //THREADEDELEMENTLOOP_H_
