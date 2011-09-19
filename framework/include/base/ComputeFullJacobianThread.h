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

#ifndef COMPUTEFULLJACOBIANTHREAD_H
#define COMPUTEFULLJACOBIANTHREAD_H

#include "ComputeJacobianThread.h"

// libMesh includes
#include "elem_range.h"

class NonlinearSystem;

class ComputeFullJacobianThread : public ComputeJacobianThread
{
public:
  ComputeFullJacobianThread(Problem & problem, NonlinearSystem & sys, SparseMatrix<Number> & jacobian);

  // Splitting Constructor
  ComputeFullJacobianThread(ComputeFullJacobianThread & x, Threads::split split);

  void join(const ComputeJacobianThread & /*y*/)
  {}


protected:
  virtual void computeJacobian();
  virtual void computeFaceJacobian(short int bnd_id);
  virtual void computeInternalFaceJacobian();
};

#endif //COMPUTEFULLJACOBIANTHREAD_H
