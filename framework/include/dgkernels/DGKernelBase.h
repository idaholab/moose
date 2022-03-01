//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// local includes
#include "MooseArray.h"
#include "NeighborResidualObject.h"
#include "BlockRestrictable.h"
#include "BoundaryRestrictable.h"
#include "NeighborCoupleableMooseVariableDependencyIntermediateInterface.h"
#include "TwoMaterialPropertyInterface.h"
#include "ElementIDInterface.h"

#include "Assembly.h"

#define usingDGKernelBaseMembers                                                                   \
  usingNeighborCoupleableMembers;                                                                  \
  usingFunctionInterfaceMembers;                                                                   \
  usingBlockRestrictableMembers;                                                                   \
  usingTransientInterfaceMembers

/**
 * Serves as a base class for DGKernel and ADDGKernel
 */
class DGKernelBase : public NeighborResidualObject,
                     public BlockRestrictable,
                     public BoundaryRestrictable,
                     public NeighborCoupleableMooseVariableDependencyIntermediateInterface,
                     public TwoMaterialPropertyInterface,
                     public ElementIDInterface
{
public:
  /**
   * Factory constructor initializes all internal references needed for residual computation.
   * @param parameters The parameters object for holding additional parameters for kernels and
   * derived kernels
   */
  static InputParameters validParams();

  DGKernelBase(const InputParameters & parameters);

  /**
   * Computes the residual for this element or the neighbor
   */
  virtual void computeElemNeighResidual(Moose::DGResidualType type) = 0;

  /**
   * Computes the residual for the current side.
   */
  virtual void computeResidual() override;

  /**
   * Computes the element/neighbor-element/neighbor Jacobian
   */
  virtual void computeElemNeighJacobian(Moose::DGJacobianType type) = 0;

  /**
   * Computes the jacobian for the current side.
   */
  virtual void computeJacobian() override;

  /**
   * Computes the element-element off-diagonal Jacobian
   */
  virtual void computeOffDiagElemNeighJacobian(Moose::DGJacobianType type,
                                               const MooseVariableFEBase & jvar) = 0;

  /**
   * Computes d-residual / d-jvar...
   */
  virtual void computeOffDiagJacobian(unsigned int jvar) override;

  void prepareShapes(unsigned int var_num) override final;

protected:
  /// Current element
  const Elem * const & _current_elem;

  /// The volume (or length) of the current element
  const Real & _current_elem_volume;

  /// The neighboring element
  const Elem * const & _neighbor_elem;

  /// The neighboring element volume
  const Real & _neighbor_elem_volume;

  /// Current side
  const unsigned int & _current_side;
  /// Current side element
  const Elem *& _current_side_elem;
  /// The volume (or length) of the current side
  const Real & _current_side_volume;

  /// Coordinate system
  const Moose::CoordinateSystemType & _coord_sys;
  /// Quadrature points
  const MooseArray<Point> & _q_point;
  /// Quadrature rule
  const QBase * const & _qrule;
  /// Jacobian det times quadrature weighting on quadrature points
  const MooseArray<Real> & _JxW;
  /// Coordinate transform mainly for curvilinear coordinates
  const MooseArray<Real> & _coord;
  /// Normal vectors at the quadrature points
  const MooseArray<Point> & _normals;

  /// The aux variables to save the residual contributions to
  bool _has_save_in;
  std::vector<MooseVariableFEBase *> _save_in;
  std::vector<AuxVariableName> _save_in_strings;

  /// The aux variables to save the diagonal Jacobian contributions to
  bool _has_diag_save_in;
  std::vector<MooseVariableFEBase *> _diag_save_in;
  std::vector<AuxVariableName> _diag_save_in_strings;

  /// The volume (or length) of the current neighbor
  const Real & getNeighborElemVolume() const { return _assembly.neighborVolume(); }

public:
  // boundary id used for internal edges (all DG kernels lives on this boundary id -- a made-up
  // number)
  static const BoundaryID InternalBndId;

protected:
  unsigned int _qp;

  unsigned int _i, _j;

  BoundaryID _boundary_id;

  static Threads::spin_mutex _resid_vars_mutex;
  static Threads::spin_mutex _jacoby_vars_mutex;

  /// Check current element if it contains broken boundary
  bool excludeBoundary() const;

private:
  /// Broken boundaries
  std::set<BoundaryID> _excluded_boundaries;

  friend class ADDGKernel;
};
