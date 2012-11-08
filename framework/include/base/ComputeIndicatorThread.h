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

#ifndef COMPUTEINDICATORTHREAD_H
#define COMPUTEINDICATORTHREAD_H

#include "ThreadedElementLoop.h"

// libMesh includes
#include "elem_range.h"

class AuxiliarySystem;

class ComputeIndicatorThread : public ThreadedElementLoop<ConstElemRange>
{
public:
  /**
   * @param finalize Whether or not we are just in the "finalize" stage or not.
   */
  ComputeIndicatorThread(FEProblem & fe_problem, AuxiliarySystem & sys, std::vector<IndicatorWarehouse> & indicator_whs, bool finalize = false);

  // Splitting Constructor
  ComputeIndicatorThread(ComputeIndicatorThread & x, Threads::split split);

  virtual ~ComputeIndicatorThread();

  virtual void subdomainChanged();
  virtual void onElement(const Elem *elem);
  virtual void onBoundary(const Elem *elem, unsigned int side, BoundaryID bnd_id);
  virtual void onInternalSide(const Elem *elem, unsigned int side);
  virtual void postElement(const Elem * /*elem*/);
  virtual void post();

  void join(const ComputeIndicatorThread & /*y*/);

protected:
  FEProblem & _fe_problem;
  AuxiliarySystem & _aux_sys;
  std::vector<IndicatorWarehouse> & _indicator_whs;
  bool _finalize;
};

#endif //COMPUTEINDICATORTHREAD_H
