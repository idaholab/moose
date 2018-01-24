//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTEJACOBIANTHREAD_H
#define COMPUTEJACOBIANTHREAD_H

#include "ThreadedElementLoop.h"

#include "libmesh/elem_range.h"

// Forward declarations
class FEProblemBase;
class NonlinearSystemBase;
class IntegratedBC;
class DGKernel;
class InterfaceKernel;
class KernelWarehouse;

class ComputeJacobianThread : public ThreadedElementLoop<ConstElemRange>
{
public:
  ComputeJacobianThread(FEProblemBase & fe_problem,
                        SparseMatrix<Number> & jacobian,
                        Moose::KernelType kernel_type = Moose::KT_ALL);

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
  NonlinearSystemBase & _nl;

  unsigned int _num_cached;

  // Reference to BC storage structures
  const MooseObjectWarehouse<IntegratedBC> & _integrated_bcs;

  // Reference to DGKernel storage structure
  const MooseObjectWarehouse<DGKernel> & _dg_kernels;

  // Reference to interface kernel storage structure
  const MooseObjectWarehouse<InterfaceKernel> & _interface_kernels;

  // Reference to Kernel storage structure
  const KernelWarehouse & _kernels;

  Moose::KernelType _kernel_type;

  virtual void computeJacobian();
  virtual void computeFaceJacobian(BoundaryID bnd_id);
  virtual void computeInternalFaceJacobian(const Elem * neighbor);
  virtual void computeInternalInterFaceJacobian(BoundaryID bnd_id);
};

#endif // COMPUTEJACOBIANTHREAD_H
