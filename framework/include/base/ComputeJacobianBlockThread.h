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

#ifndef COMPUTEJACOBIANBLOCKTHREAD_H
#define COMPUTEJACOBIANBLOCKTHREAD_H

#include "ThreadedElementLoop.h"
// libMesh includes
#include "libmesh/elem_range.h"

class ComputeJacobianBlockThread
{
public:
  ComputeJacobianBlockThread(FEProblem & fe_problem, libMesh::System & precond_system, SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar);
  ComputeJacobianBlockThread(ComputeJacobianBlockThread & x, Threads::split split);
  virtual ~ComputeJacobianBlockThread();

  void operator() (const ConstElemRange & range, bool bypass_threading=false);

  void join(const ComputeJacobianBlockThread & /*y*/);

protected:
  THREAD_ID _tid;

  FEProblem & _fe_problem;
  libMesh::System & _precond_system;
  NonlinearSystem & _nl;

  MooseMesh & _mesh;
  SparseMatrix<Number> & _jacobian;
  unsigned int _ivar, _jvar;
};

#endif /* COMPUTEJACOBIANBLOCKTHREAD_H */
