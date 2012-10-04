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

#ifndef COMPUTEUSEROBJECTSTHREAD_H
#define COMPUTEUSEROBJECTSTHREAD_H

#include "ThreadedElementLoop.h"
#include "UserObjectWarehouse.h"

// libMesh includes
#include "elem_range.h"
#include "numeric_vector.h"

//
class ComputeUserObjectsThread : public ThreadedElementLoop<ConstElemRange>
{
public:
  ComputeUserObjectsThread(FEProblem & problem, SystemBase & sys, const NumericVector<Number>& in_soln, std::vector<UserObjectWarehouse> & user_objects, UserObjectWarehouse::GROUP);
  ComputeUserObjectsThread(ComputeUserObjectsThread & x, Threads::split);                 // Splitting Constructor

  virtual void onElement(const Elem *elem);
  virtual void onBoundary(const Elem *elem, unsigned int side, BoundaryID bnd_id);
  virtual void post();
  virtual void subdomainChanged();

  void join(const ComputeUserObjectsThread & /*y*/);

protected:
  const NumericVector<Number>& _soln;
  std::vector<UserObjectWarehouse> & _user_objects;
  UserObjectWarehouse::GROUP _group;
};

#endif //COMPUTEUSEROBJECTSTHREAD_H
