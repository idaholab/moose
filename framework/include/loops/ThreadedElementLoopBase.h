//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ParallelUniqueId.h"
#include "MooseMesh.h"
#include "MooseTypes.h"
#include "MooseException.h"
#include "libmesh/libmesh_exceptions.h"
#include "libmesh/elem.h"

/**
 * Base class for assembly-like calculations.
 */
template <typename RangeType>
class ThreadedElementLoopBase
{
public:
  ThreadedElementLoopBase(MooseMesh & mesh);

  ThreadedElementLoopBase(ThreadedElementLoopBase & x, Threads::split split);

  virtual ~ThreadedElementLoopBase();

  virtual void operator()(const RangeType & range, bool bypass_threading = false);

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
  virtual void onElement(const Elem * elem);

  /**
   * Called before the element assembly
   *
   * @param elem - active element
   */
  virtual void preElement(const Elem * elem);

  /**
   * Called after the element assembly is done (including surface assembling)
   *
   * @param elem - active element
   */
  virtual void postElement(const Elem * elem);

  /**
   * Called before the boundary assembly
   *
   * @param elem - The element we are checking is on the boundary.
   * @param side - The side of the element in question.
   * @param bnd_id - ID of the boundary we are at
   * @param lower_d_elem - Lower dimensional element (e.g. Mortar)
   */
  virtual void preBoundary(const Elem * elem,
                           unsigned int side,
                           BoundaryID bnd_id,
                           const Elem * lower_d_elem = nullptr);

  /**
   * Called when doing boundary assembling
   *
   * @param elem - The element we are checking is on the boundary.
   * @param side - The side of the element in question.
   * @param bnd_id - ID of the boundary we are at
   * @param lower_d_elem - Lower dimensional element (e.g. Mortar)
   */
  virtual void onBoundary(const Elem * elem,
                          unsigned int side,
                          BoundaryID bnd_id,
                          const Elem * lower_d_elem = nullptr);

  /**
   * Called before evaluations on an element internal side
   *
   * @param elem - Element we are on
   * @param side - local side number of the element 'elem'
   */
  virtual void preInternalSide(const Elem * elem, unsigned int side);

  /**
   * Called after evaluations on an element internal side
   *
   * @param elem - Element we are on
   * @param side - local side number of the element 'elem'
   */
  virtual void postInternalSide(const Elem * elem, unsigned int side);

  /**
   * Called when doing internal edge assembling
   *
   * @param elem - Element we are on
   * @param side - local side number of the element 'elem'
   */
  virtual void onInternalSide(const Elem * elem, unsigned int side);

  /**
   * Called when doing interface assembling
   *
   * @param elem - Element we are on
   * @param side - local side number of the element 'elem'
   * @param bnd_id - ID of the interface we are at
   */
  virtual void onInterface(const Elem * elem, unsigned int side, BoundaryID bnd_id);

  /**
   * Called every time the current subdomain changes (i.e. the subdomain of _this_ element
   * is not the same as the subdomain of the last element).  Beware of over-using this!
   * You might think that you can do some expensive stuff in here and get away with it...
   * but there are applications that have TONS of subdomains....
   */
  virtual void subdomainChanged();

  /**
   * Called every time the neighbor subdomain changes (i.e. the subdomain of _this_ neighbor
   * is not the same as the subdomain of the last neighbor).  Beware of over-using this!
   * You might think that you can do some expensive stuff in here and get away with it...
   * but there are applications that have TONS of subdomains....
   */
  virtual void neighborSubdomainChanged();

  /**
   * Called if a MooseException is caught anywhere during the computation.
   * The single input parameter taken is a MooseException object.
   */
  virtual void caughtMooseException(MooseException &){};

  /**
   * Whether or not the loop should continue.
   *
   * @return true to keep going, false to stop.
   */
  virtual bool keepGoing() { return true; }

protected:
  MooseMesh & _mesh;
  THREAD_ID _tid;

  /// The subdomain for the current element
  SubdomainID _subdomain;

  /// The subdomain for the last element
  SubdomainID _old_subdomain;

  /// The subdomain for the current neighbor
  SubdomainID _neighbor_subdomain;

  /// The subdomain for the last neighbor
  SubdomainID _old_neighbor_subdomain;
};

template <typename RangeType>
ThreadedElementLoopBase<RangeType>::ThreadedElementLoopBase(MooseMesh & mesh) : _mesh(mesh)
{
}

template <typename RangeType>
ThreadedElementLoopBase<RangeType>::ThreadedElementLoopBase(ThreadedElementLoopBase & x,
                                                            Threads::split /*split*/)
  : _mesh(x._mesh)
{
}

template <typename RangeType>
ThreadedElementLoopBase<RangeType>::~ThreadedElementLoopBase()
{
}

template <typename RangeType>
void
ThreadedElementLoopBase<RangeType>::operator()(const RangeType & range, bool bypass_threading)
{
  try
  {
    try
    {
      ParallelUniqueId puid;
      _tid = bypass_threading ? 0 : puid.id;

      pre();

      _subdomain = Moose::INVALID_BLOCK_ID;
      _neighbor_subdomain = Moose::INVALID_BLOCK_ID;
      typename RangeType::const_iterator el = range.begin();
      for (el = range.begin(); el != range.end(); ++el)
      {
        if (!keepGoing())
          break;

        const Elem * elem = *el;

        preElement(elem);

        _old_subdomain = _subdomain;
        _subdomain = elem->subdomain_id();
        if (_subdomain != _old_subdomain)
          subdomainChanged();

        onElement(elem);

        for (unsigned int side = 0; side < elem->n_sides(); side++)
        {
          std::vector<BoundaryID> boundary_ids = _mesh.getBoundaryIDs(elem, side);
          const Elem * lower_d_elem = _mesh.getLowerDElem(elem, side);

          if (boundary_ids.size() > 0)
            for (std::vector<BoundaryID>::iterator it = boundary_ids.begin();
                 it != boundary_ids.end();
                 ++it)
            {
              preBoundary(elem, side, *it, lower_d_elem);
              onBoundary(elem, side, *it, lower_d_elem);
            }

          const Elem * neighbor = elem->neighbor_ptr(side);
          if (neighbor != nullptr)
          {
            preInternalSide(elem, side);

            _old_neighbor_subdomain = _neighbor_subdomain;
            _neighbor_subdomain = neighbor->subdomain_id();
            if (_neighbor_subdomain != _old_neighbor_subdomain)
              neighborSubdomainChanged();

            onInternalSide(elem, side);

            if (boundary_ids.size() > 0)
              for (std::vector<BoundaryID>::iterator it = boundary_ids.begin();
                   it != boundary_ids.end();
                   ++it)
                onInterface(elem, side, *it);

            postInternalSide(elem, side);
          }
        } // sides
        postElement(elem);

      } // range

      post();
    }
    catch (libMesh::LogicError & e)
    {
      throw MooseException("We caught a libMesh error in ThreadedElementLoopBase");
    }
  }
  catch (MooseException & e)
  {
    caughtMooseException(e);
  }
}

template <typename RangeType>
void
ThreadedElementLoopBase<RangeType>::pre()
{
}

template <typename RangeType>
void
ThreadedElementLoopBase<RangeType>::post()
{
}

template <typename RangeType>
void
ThreadedElementLoopBase<RangeType>::onElement(const Elem * /*elem*/)
{
}

template <typename RangeType>
void
ThreadedElementLoopBase<RangeType>::preElement(const Elem * /*elem*/)
{
}

template <typename RangeType>
void
ThreadedElementLoopBase<RangeType>::postElement(const Elem * /*elem*/)
{
}

template <typename RangeType>
void
ThreadedElementLoopBase<RangeType>::preBoundary(const Elem * /*elem*/,
                                                unsigned int /*side*/,
                                                BoundaryID /*bnd_id*/,
                                                const Elem * /*lower_d_elem = nullptr*/)
{
}

template <typename RangeType>
void
ThreadedElementLoopBase<RangeType>::onBoundary(const Elem * /*elem*/,
                                               unsigned int /*side*/,
                                               BoundaryID /*bnd_id*/,
                                               const Elem * /*lower_d_elem = nullptr*/)
{
}

template <typename RangeType>
void
ThreadedElementLoopBase<RangeType>::preInternalSide(const Elem * /*elem*/, unsigned int /*side*/)
{
}

template <typename RangeType>
void
ThreadedElementLoopBase<RangeType>::postInternalSide(const Elem * /*elem*/, unsigned int /*side*/)
{
}

template <typename RangeType>
void
ThreadedElementLoopBase<RangeType>::onInternalSide(const Elem * /*elem*/, unsigned int /*side*/)
{
}

template <typename RangeType>
void
ThreadedElementLoopBase<RangeType>::onInterface(const Elem * /*elem*/,
                                                unsigned int /*side*/,
                                                BoundaryID /*bnd_id*/)
{
}

template <typename RangeType>
void
ThreadedElementLoopBase<RangeType>::subdomainChanged()
{
}

template <typename RangeType>
void
ThreadedElementLoopBase<RangeType>::neighborSubdomainChanged()
{
}
