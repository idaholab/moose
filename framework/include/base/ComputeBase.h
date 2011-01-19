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

#ifndef COMPUTEBASE_H
#define COMPUTEBASE_H

#include "Moose.h"
#include "MooseSystem.h"
#include "ParallelUniqueId.h"

//libmesh includes
#include "boundary_info.h"

class MooseSystem;

/**
 * Base class for assembling-like calculations
 *
 */
template<typename RangeType>
class ComputeBase
{
public:
  ComputeBase(MooseSystem &moose_system);

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
  MooseSystem &_moose_system;

  THREAD_ID _tid;
};


template<typename RangeType>
ComputeBase<RangeType>::ComputeBase(MooseSystem &sys) :
  _moose_system(sys)
{
}

template<typename RangeType>
void
ComputeBase<RangeType>::operator () (const RangeType & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  pre();

  bool calculate_element_time = _moose_system._calculate_element_time;

  unsigned int subdomain = std::numeric_limits<unsigned int>::max();
  typename RangeType::const_iterator el = range.begin();
  for (el = range.begin() ; el != range.end(); ++el)
  {
    const Elem* elem = *el;
    unsigned int cur_subdomain = elem->subdomain_id();

    if(unlikely(calculate_element_time))
      _moose_system.startElementTiming(elem->id());

    preElement(elem);

    if(cur_subdomain != subdomain)
    {
      subdomain = cur_subdomain;
      onDomainChanged(subdomain);
    }

    onElement(elem);

    for (unsigned int side=0; side<elem->n_sides(); side++)
    {
      std::vector<short int> boundary_ids = _moose_system.getMesh()->boundary_info->boundary_ids (elem, side);

      if (boundary_ids.size() > 0)
      {
        for (std::vector<short int>::iterator it = boundary_ids.begin(); it != boundary_ids.end(); ++it)
          onBoundary(elem, side, *it);
      }

      if (elem->neighbor(side) != NULL)
        onInternalSide(elem, side);
    } // sides

    postElement(elem);
    
    if(unlikely(calculate_element_time))
      _moose_system.stopElementTiming(elem->id());
  } // range

  post();
}

template<typename RangeType>
void
ComputeBase<RangeType>::pre()
{

}

template<typename RangeType>
void
ComputeBase<RangeType>::post()
{

}

template<typename RangeType>
void
ComputeBase<RangeType>::preElement(const Elem * /*elem*/)
{
}

template<typename RangeType>
void
ComputeBase<RangeType>::onElement(const Elem * /*elem*/)
{
}

template<typename RangeType>
void
ComputeBase<RangeType>::postElement(const Elem * /*elem*/)
{
}

template<typename RangeType>
void
ComputeBase<RangeType>::onDomainChanged(short int /*subdomain*/)
{
}

template<typename RangeType>
void
ComputeBase<RangeType>::onBoundary(const Elem * /*elem*/, unsigned int /*side*/, short int /*bnd_id*/)
{
}

template<typename RangeType>
void
ComputeBase<RangeType>::onInternalSide(const Elem * /*elem*/, unsigned int /*side*/)
{
}

#endif //COMPUTEBASE_H
