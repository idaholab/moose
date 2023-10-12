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
class LinearSystem;
class ResidualObject;
class FVElementalKernel;

class LinearThread : public ThreadedElementLoop<ConstElemRange>
{
public:
  LinearThread(FEProblemBase & fe_problem);

  // Splitting Constructor
  LinearThread(LinearThread & x, Threads::split split);

  virtual ~LinearThread();

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
  virtual void compute(ResidualObject & ro) {}

  ///@{
  /// Defaults to forwarding to the residual object class
  // virtual void compute(KernelBase & kernel);
  virtual void compute(FVElementalKernel & kernel);
  // virtual void compute(IntegratedBCBase & bc);
  // virtual void compute(DGKernelBase & dg, const Elem * neighbor);
  // virtual void compute(InterfaceKernelBase & ik);
  ///@}

  /**
   * Add neighbor residual/Jacobian into assembly global data
   */
  virtual void accumulateNeighbor() {}

  /**
   * Add neighbor and lower residual/Jacobian into assembly global data
   */
  virtual void accumulateNeighborLower() {}

  /**
   * Add lower-d residual/Jacobian into assembly global data
   */
  virtual void accumulateLower() {}

  /**
   * Add element residual/Jacobian into assembly global data
   */
  virtual void accumulate() {}

  /**
   * Determine the objects we will actually compute based on vector/matrix tag information
   */
  virtual void determineObjectWarehouses() {}

  /// Return what the loops is meant to compute
  virtual std::string objectType() const { return ""; };

  LinearSystem & _linear_system;

  /// Whether there are any active residual objects; otherwise we will do an early return
  const bool _has_active_objects;
};
