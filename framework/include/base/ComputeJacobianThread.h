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

#ifndef COMPUTEJACOBIANTHREAD_H
#define COMPUTEJACOBIANTHREAD_H

#include "ThreadedElementLoop.h"

// libMesh includes
#include "libmesh/elem_range.h"

// Forward declarations
class FEProblem;
class NonlinearSystemBase;
class IntegratedBC;
class DGKernel;
class InterfaceKernel;
class KernelWarehouse;

class ComputeJacobianThread : public ThreadedElementLoop<ConstElemRange>
{
public:
  ComputeJacobianThread(FEProblem & fe_problem, NonlinearSystemBase & sys, SparseMatrix<Number> & jacobian);

  // Splitting Constructor
  ComputeJacobianThread(ComputeJacobianThread & x, Threads::split split);

  virtual ~ComputeJacobianThread();

  virtual void subdomainChanged() override;
  virtual void onElement(const Elem * elem) override;
  virtual void onBoundary(const Elem * elem, unsigned int side, BoundaryID bnd_id) override;
  virtual void onInternalSide(const Elem * elem, unsigned int side) override;
  virtual void onInterface(const Elem * elem, unsigned int side, BoundaryID bnd_id) override;
  virtual void postElement(const Elem * /*elem*/) override;
  virtual void post() override;

  void join(const ComputeJacobianThread & /*y*/);

protected:
  SparseMatrix<Number> & _jacobian;
  NonlinearSystemBase & _sys;

  unsigned int _num_cached;

  // Reference to BC storage structures
  const MooseObjectWarehouse<IntegratedBC> & _integrated_bcs;

  // Reference to DGKernel storage structure
  const MooseObjectWarehouse<DGKernel> & _dg_kernels;

  // Reference to interface kernel storage structure
  const MooseObjectWarehouse<InterfaceKernel> & _interface_kernels;

  // Reference to Kernel storage structure
  const KernelWarehouse & _kernels;

  virtual void computeJacobian();
  virtual void computeFaceJacobian(BoundaryID bnd_id);
  virtual void computeInternalFaceJacobian(const Elem * neighbor);
  virtual void computeInternalInterFaceJacobian(BoundaryID bnd_id);
};

#endif //COMPUTEJACOBIANTHREAD_H
