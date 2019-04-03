//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DGKERNELBASE_H
#define DGKERNELBASE_H

// local includes
#include "MooseArray.h"
#include "MooseObject.h"
#include "BlockRestrictable.h"
#include "BoundaryRestrictable.h"
#include "SetupInterface.h"
#include "TransientInterface.h"
#include "UserObjectInterface.h"
#include "NeighborCoupleableMooseVariableDependencyIntermediateInterface.h"
#include "FunctionInterface.h"
#include "TwoMaterialPropertyInterface.h"
#include "Restartable.h"
#include "MeshChangedInterface.h"
#include "TaggingInterface.h"

// Forward Declarations
class MooseMesh;
class SubProblem;
class Assembly;

class DGKernelBase;

#define usingDGKernelBaseMembers                                                                   \
  usingNeighborCoupleableMembers;                                                                  \
  usingFunctionInterfaceMembers;                                                                   \
  usingBlockRestrictableMembers;                                                                   \
  usingTransientInterfaceMembers

template <>
InputParameters validParams<DGKernelBase>();

/**
 * Serves as a base class for DGKernel and ADDGKernel
 */
class DGKernelBase : public MooseObject,
                     public BlockRestrictable,
                     public BoundaryRestrictable,
                     public SetupInterface,
                     public TransientInterface,
                     public FunctionInterface,
                     public UserObjectInterface,
                     public NeighborCoupleableMooseVariableDependencyIntermediateInterface,
                     public NeighborMooseVariableInterface<Real>,
                     public TwoMaterialPropertyInterface,
                     public Restartable,
                     public MeshChangedInterface,
                     public TaggingInterface
{
public:
  /**
   * Factory constructor initializes all internal references needed for residual computation.
   * @param parameters The parameters object for holding additional parameters for kernels and
   * derived kernels
   */
  DGKernelBase(const InputParameters & parameters);

  virtual ~DGKernelBase();

  /**
   * The variable number that this kernel operates on.
   */
  virtual MooseVariable & variable();

  /**
   * Return a reference to the subproblem.
   */
  SubProblem & subProblem();

  /**
   * Computes the residual for this element or the neighbor
   */
  virtual void computeElemNeighResidual(Moose::DGResidualType type) = 0;

  /**
   * Computes the residual for the current side.
   */
  virtual void computeResidual();

  /**
   * Computes the element/neighbor-element/neighbor Jacobian
   */
  virtual void computeElemNeighJacobian(Moose::DGJacobianType type) = 0;

  /**
   * Computes the jacobian for the current side.
   */
  virtual void computeJacobian();

  /**
   * Computes the element-element off-diagonal Jacobian
   */
  virtual void computeOffDiagElemNeighJacobian(Moose::DGJacobianType type, unsigned int jvar) = 0;

  /**
   * Computes d-residual / d-jvar...
   */
  virtual void computeOffDiagJacobian(unsigned int jvar);

protected:
  SubProblem & _subproblem;
  SystemBase & _sys;

  THREAD_ID _tid;

  Assembly & _assembly;
  MooseVariable & _var;
  MooseMesh & _mesh;

  const Elem *& _current_elem;

  /// The volume (or length) of the current element
  const Real & _current_elem_volume;

  /// The neighboring element
  const Elem *& _neighbor_elem;

  /// Current side
  unsigned int & _current_side;
  /// Current side element
  const Elem *& _current_side_elem;

  /// The volume (or length) of the current side
  const Real & _current_side_volume;

  /// Coordinate system
  const Moose::CoordinateSystemType & _coord_sys;
  unsigned int _qp;
  const MooseArray<Point> & _q_point;
  QBase *& _qrule;
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;

  unsigned int _i, _j;

  BoundaryID _boundary_id;

  /// Holds the current solution at the current quadrature point on the face.
  const VariableValue & _u;

  /// Holds the current solution gradient at the current quadrature point on the face.
  const VariableGradient & _grad_u;
  // shape functions
  const VariablePhiValue & _phi;
  const VariablePhiGradient & _grad_phi;
  // test functions

  /// Side shape function.
  const VariableTestValue & _test;
  /// Gradient of side shape function
  const VariableTestGradient & _grad_test;
  /// Normal vectors at the quadrature points
  const MooseArray<Point> & _normals;

  /// Side shape function.
  const VariablePhiValue & _phi_neighbor;
  /// Gradient of side shape function
  const VariablePhiGradient & _grad_phi_neighbor;

  /// Side test function
  const VariableTestValue & _test_neighbor;
  /// Gradient of side shape function
  const VariableTestGradient & _grad_test_neighbor;

  /// Holds the current solution at the current quadrature point
  const VariableValue & _u_neighbor;
  /// Holds the current solution gradient at the current quadrature point
  const VariableGradient & _grad_u_neighbor;

  /// The aux variables to save the residual contributions to
  bool _has_save_in;
  std::vector<MooseVariableFEBase *> _save_in;
  std::vector<AuxVariableName> _save_in_strings;

  /// The aux variables to save the diagonal Jacobian contributions to
  bool _has_diag_save_in;
  std::vector<MooseVariableFEBase *> _diag_save_in;
  std::vector<AuxVariableName> _diag_save_in_strings;

  /// The volume (or length) of the current neighbor
  const Real & getNeighborElemVolume();

public:
  // boundary id used for internal edges (all DG kernels lives on this boundary id -- a made-up
  // number)
  static const BoundaryID InternalBndId;

protected:
  static Threads::spin_mutex _resid_vars_mutex;
  static Threads::spin_mutex _jacoby_vars_mutex;
};

#endif // DGKERNELBASE_H
