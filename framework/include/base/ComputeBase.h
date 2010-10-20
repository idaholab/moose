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

#include "elem_range.h"

#include "Moose.h"
#include "ParallelUniqueId.h"

class MooseSystem;

/**
 * Base class for assembling-like calculations
 *
 */
class ComputeBase
{
public:
  ComputeBase(MooseSystem &moose_system);

  void operator() (const ConstElemRange & range);

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

#endif //COMPUTEBASE_H
