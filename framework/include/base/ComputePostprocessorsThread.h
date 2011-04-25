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

#ifndef COMPUTEPOSTPROCESSORSTHREAD_H
#define COMPUTEPOSTPROCESSORSTHREAD_H

#include "ThreadedElementLoop.h"
#include "PostprocessorWarehouse.h"

// libMesh includes
#include "elem_range.h"
#include "numeric_vector.h"

//
class ComputePostprocessorsThread : public ThreadedElementLoop<ConstElemRange>
{
public:
  ComputePostprocessorsThread(Problem & problem, SystemBase & sys, const NumericVector<Number>& in_soln, std::vector<PostprocessorWarehouse> & pps);
  ComputePostprocessorsThread(ComputePostprocessorsThread & x, Threads::split);                 // Splitting Constructor

  virtual void onElement(const Elem *elem);
  virtual void onBoundary(const Elem *elem, unsigned int side, short int bnd_id);

  void join(const ComputePostprocessorsThread & /*y*/);

protected:
  const NumericVector<Number>& _soln;
  std::vector<PostprocessorWarehouse> & _pps;
};

#endif //COMPUTEPOSTPROCESSORSTHREAD_H
