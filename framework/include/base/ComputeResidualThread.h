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
#include "elem_range.h"

class NonlinearSystem;


class ComputeResidualThread : public ThreadedElementLoop<ConstElemRange>
{
public:
  ComputeResidualThread(Problem & problem, NonlinearSystem & sys, NumericVector<Number> & residual);  

  // Splitting Constructor
  ComputeResidualThread(ComputeResidualThread & x, Threads::split split);

  virtual void onElement(const Elem *elem);
  virtual void onBoundary(const Elem *elem, unsigned int side, short int bnd_id);
  virtual void postElement(const Elem * /*elem*/);

  void join(const ComputeResidualThread & /*y*/);

protected:
  NumericVector<Number> & _residual;
  NonlinearSystem & _sys;
  std::set<MooseVariable *> _vars;
};

#endif //COMPUTERESIDUALTHREAD_H
