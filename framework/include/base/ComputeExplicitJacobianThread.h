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

#ifndef COMPUTEEXPLICITJACOBIANTHREAD_H
#define COMPUTEEXPLICITJACOBIANTHREAD_H

#include "ComputeJacobianThread.h"
// libMesh includes
#include "libmesh/elem_range.h"

class NonlinearSystem;

class ComputeExplicitJacobianThread : public ComputeJacobianThread
{
public:
  ComputeExplicitJacobianThread(FEProblem & fe_problem, NonlinearSystem & sys, SparseMatrix<Number> & jacobian);

  // Splitting Constructor
  ComputeExplicitJacobianThread(ComputeExplicitJacobianThread & x, Threads::split split);

  virtual ~ComputeExplicitJacobianThread();

  void join(const ComputeJacobianThread & /*y*/)
  {}


protected:
  virtual void computeJacobian();
};

#endif //COMPUTEEXPLICITJACOBIANTHREAD_H
