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

#ifndef COMPUTERESIDUALTHREAD_H
#define COMPUTERESIDUALTHREAD_H

#include "ThreadedElementLoop.h"
// libMesh includes
#include "libmesh/elem_range.h"


class FEProblem;
class NonlinearSystem;


class ComputeResidualThread : public ThreadedElementLoop<ConstElemRange>
{
public:
  ComputeResidualThread(FEProblem & fe_problem, NonlinearSystem & sys, Moose::KernelType type);
  // Splitting Constructor
  ComputeResidualThread(ComputeResidualThread & x, Threads::split split);

  virtual ~ComputeResidualThread();

  virtual void subdomainChanged();
  virtual void onElement(const Elem *elem );
  virtual void onBoundary(const Elem *elem, unsigned int side, BoundaryID bnd_id);
  virtual void onInternalSide(const Elem *elem, unsigned int side);
  virtual void postElement(const Elem * /*elem*/);
  virtual void post();

  void join(const ComputeResidualThread & /*y*/);

protected:
  NonlinearSystem & _sys;
  Moose::KernelType _kernel_type;
  unsigned int _num_cached;
};

#endif //COMPUTERESIDUALTHREAD_H
