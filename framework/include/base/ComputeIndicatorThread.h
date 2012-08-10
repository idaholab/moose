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
  ComputeIndicatorThread(FEProblem & fe_problem, AuxiliarySystem & sys, std::vector<IndicatorWarehouse> & indicator_whs);

  // Splitting Constructor
  ComputeIndicatorThread(ComputeIndicatorThread & x, Threads::split split);

  virtual void onElement(const Elem *elem);
  virtual void onBoundary(const Elem *elem, unsigned int side, BoundaryID bnd_id);
  virtual void onInternalSide(const Elem *elem, unsigned int side);
  virtual void postElement(const Elem * /*elem*/);

  void join(const ComputeIndicatorThread & /*y*/);

protected:
  FEProblem & _fe_problem;
  std::vector<IndicatorWarehouse> & _indicator_whs;
};

#endif //COMPUTEINDICATORTHREAD_H
