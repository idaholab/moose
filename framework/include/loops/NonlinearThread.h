//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ThreadedElementLoop.h"
#include "MooseObjectTagWarehouse.h"

#include "libmesh/elem_range.h"

// Forward declarations
class FEProblemBase;
class NonlinearSystemBase;
class IntegratedBCBase;
class DGKernelBase;
class InterfaceKernelBase;
class TimeKernel;
class KernelBase;
class Kernel;
class ResidualObject;
class FVElementalKernel;

class NonlinearThread : public ThreadedElementLoop<ConstElemRange>
{
public:
  NonlinearThread(FEProblemBase & fe_problem);

  // Splitting Constructor
  NonlinearThread(NonlinearThread & x, Threads::split split);

  virtual ~NonlinearThread();

  virtual void operator()(const ConstElemRange & range, bool bypass_threading = false) override;

  virtual void subdomainChanged() override;
  virtual void onElement(const Elem * elem) override;
  virtual void onBoundary(const Elem * elem,
                          unsigned int side,
                          BoundaryID bnd_id,
                          const Elem * lower_d_elem = nullptr) override;
  virtual void onInterface(const Elem * elem, unsigned int side, BoundaryID bnd_id) override;
  virtual void onInternalSide(const Elem * elem, unsigned int side) override;
  virtual void postElement(const Elem * /*elem*/) override;
  virtual void post() override;

protected:
  ///@{
  /// Base class version just calls compute on each object for the element
  virtual void computeOnElement();
  virtual void computeOnBoundary(BoundaryID bnd_id, const Elem * lower_d_elem);
  virtual void computeOnInterface(BoundaryID bnd_id);
  virtual void computeOnInternalFace(const Elem * neighbor);
  ///@}

  /**
   * Will dispatch to computeResidual/computeJacobian/computeResidualAndJacobian based on the
   * derived class
   */
  virtual void compute(ResidualObject & ro) = 0;

  ///@{
  /// Defaults to forwarding to the residual object class
  virtual void compute(KernelBase & kernel);
  virtual void compute(FVElementalKernel & kernel);
  virtual void compute(IntegratedBCBase & bc);
  virtual void compute(DGKernelBase & dg, const Elem * neighbor);
  virtual void compute(InterfaceKernelBase & ik);
  ///@}

  /**
   * Add neighbor residual/Jacobian into assembly global data
   */
  virtual void accumulateNeighbor() = 0;

  /**
   * Add neighbor and lower residual/Jacobian into assembly global data
   */
  virtual void accumulateNeighborLower() = 0;

  /**
   * Add lower-d residual/Jacobian into assembly global data
   */
  virtual void accumulateLower() = 0;

  /**
   * Add element residual/Jacobian into assembly global data
   */
  virtual void accumulate() = 0;

  /**
   * Determine the objects we will actually compute based on vector/matrix tag information
   */
  virtual void determineObjectWarehouses() = 0;

  /// Print information about the loop, mostly order of execution of objects
  void printGeneralExecutionInformation() const override;

  /// Print list of specific objects executed on each block and in which order
  void printBlockExecutionInformation() const override;

  /// Print list of specific objects executed on each boundary and in which order
  void printBoundaryExecutionInformation(const unsigned int bid) const override;

  /// Return what the loops is meant to compute
  virtual std::string objectType() const { return ""; };

  NonlinearSystemBase & _nl;
  unsigned int _num_cached;

  /// Reference to BC storage structures
  MooseObjectTagWarehouse<IntegratedBCBase> & _integrated_bcs;

  MooseObjectWarehouse<IntegratedBCBase> * _ibc_warehouse;

  /// Reference to DGKernel storage structure
  MooseObjectTagWarehouse<DGKernelBase> & _dg_kernels;

  MooseObjectWarehouse<DGKernelBase> * _dg_warehouse;

  /// Reference to interface kernel storage structure
  MooseObjectTagWarehouse<InterfaceKernelBase> & _interface_kernels;

  MooseObjectWarehouse<InterfaceKernelBase> * _ik_warehouse;

  ///@{
  /// Reference to Kernel storage structures
  MooseObjectTagWarehouse<KernelBase> & _kernels;

  MooseObjectWarehouse<KernelBase> * _tag_kernels;
  ///@}

  /// Current subdomain FVElementalKernels
  std::vector<FVElementalKernel *> _fv_kernels;

  /// Whether there are any active residual objects; otherwise we will do an early return
  const bool _has_active_objects;
};
