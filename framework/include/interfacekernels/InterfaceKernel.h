//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INTERFACEKERNEL_H
#define INTERFACEKERNEL_H

// local includes
#include "MooseArray.h"
#include "MooseObject.h"
#include "BoundaryRestrictable.h"
#include "SetupInterface.h"
#include "TransientInterface.h"
#include "UserObjectInterface.h"
#include "NeighborCoupleableMooseVariableDependencyIntermediateInterface.h"
#include "FunctionInterface.h"
#include "Restartable.h"
#include "MeshChangedInterface.h"
#include "TwoMaterialPropertyInterface.h"

// Forward Declarations
class InterfaceKernel;

template <>
InputParameters validParams<InterfaceKernel>();

/**
 * InterfaceKernel is responsible for interfacing physics across subdomains
 */

class InterfaceKernel : public MooseObject,
                        public BoundaryRestrictable,
                        public SetupInterface,
                        public TransientInterface,
                        public FunctionInterface,
                        public UserObjectInterface,
                        public NeighborCoupleableMooseVariableDependencyIntermediateInterface,
                        public Restartable,
                        public MeshChangedInterface,
                        public TwoMaterialPropertyInterface
{
public:
  InterfaceKernel(const InputParameters & parameters);

  /// The master variable that this interface kernel operates on
  MooseVariable & variable() const;

  /// The neighbor variable number that this interface kernel operates on
  const MooseVariable & neighborVariable() const;

  /// Return a reference to the subproblem.
  SubProblem & subProblem();

  /**
   * Using the passed DGResidual type, selects the correct test function space and residual block,
   * and then calls computeQpResidual
   */
  virtual void computeElemNeighResidual(Moose::DGResidualType type);

  /**
   * Using the passed DGJacobian type, selects the correct test function and trial function spaces
   * and
   * jacobian block, and then calls computeQpJacobian
   */
  virtual void computeElemNeighJacobian(Moose::DGJacobianType type);

  /**
   * Using the passed DGJacobian type, selects the correct test function and trial function spaces
   * and
   * jacobian block, and then calls computeQpOffDiagJacobian with the passed jvar
   */
  virtual void computeOffDiagElemNeighJacobian(Moose::DGJacobianType type, unsigned int jvar);

  /// Selects the correct Jacobian type and routine to call for the master variable jacobian
  virtual void computeElementOffDiagJacobian(unsigned int jvar);

  /// Selects the correct Jacobian type and routine to call for the slave variable jacobian
  virtual void computeNeighborOffDiagJacobian(unsigned int jvar);

  /// Computes the residual for the current side.
  virtual void computeResidual();

  /// Computes the jacobian for the current side.
  virtual void computeJacobian();

protected:
  /// Compute residuals at quadrature points
  virtual Real computeQpResidual(Moose::DGResidualType type) = 0;

  /// Compute jacobians at quadrature points
  virtual Real computeQpJacobian(Moose::DGJacobianType type) = 0;

  /// compute off-diagonal jacobian components at quadrature points
  virtual Real computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar);

  /// The volume of the current neighbor
  const Real & getNeighborElemVolume();

  /// Reference to the controlling finite element problem
  SubProblem & _subproblem;

  /// Reference to the nonlinear system
  SystemBase & _sys;

  /// The thread ID
  THREAD_ID _tid;

  /// Problem assembly
  Assembly & _assembly;

  /// The master side MooseVariable
  MooseVariable & _var;

  /// The problem mesh
  MooseMesh & _mesh;

  /// Pointer reference to the current element
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

  /// Current quadrature point
  unsigned int _qp;

  /// Array that holds element quadrature point coordinates
  const MooseArray<Point> & _q_point;

  /// Quadrature rule
  QBase *& _qrule;

  /// Elemtn Jacobian/quadrature weight
  const MooseArray<Real> & _JxW;

  /// Coordinate transformation value; relevant in axisymmetric simulations for example
  const MooseArray<Real> & _coord;

  /// Index for test and trial functions
  unsigned int _i, _j;

  /// Holds the current solution at the current quadrature point on the face.
  const VariableValue & _u;

  /// Holds the current solution gradient at the current quadrature point on the face.
  const VariableGradient & _grad_u;

  /// shape function
  const VariablePhiValue & _phi;

  /// Shape function gradient
  const VariablePhiGradient & _grad_phi;

  /// Side shape function.
  const VariableTestValue & _test;
  /// Gradient of side shape function
  const VariableTestGradient & _grad_test;
  /// Normal vectors at the quadrature points
  const MooseArray<Point> & _normals;

  /// Coupled neighbor variable
  MooseVariable & _neighbor_var;

  /// Coupled neighbor variable value
  const VariableValue & _neighbor_value;
  /// Coupled neighbor variable gradient
  const VariableGradient & _grad_neighbor_value;

  /// Side neighbor shape function.
  const VariablePhiValue & _phi_neighbor;
  /// Gradient of side neighbor shape function
  const VariablePhiGradient & _grad_phi_neighbor;

  /// Side neighbor test function
  const VariableTestValue & _test_neighbor;
  /// Gradient of side neighbor shape function
  const VariableTestGradient & _grad_test_neighbor;

  /// Holds residual entries as they are accumulated by this InterfaceKernel
  DenseVector<Number> _local_re;

  /// Holds residual entries as they are accumulated by this InterfaceKernel
  DenseMatrix<Number> _local_kxx;

  /** MultiMooseEnum specifying whether residual save-in
   * aux variables correspond to master or slave side
   */
  MultiMooseEnum _save_in_var_side;

  /** The names of the aux variables that will be used to save-in residuals
   * (includes both master and slave variable names)
   */
  std::vector<AuxVariableName> _save_in_strings;

  /// Whether there are master residual aux variables
  bool _has_master_residuals_saved_in;

  /// The aux variables to save the master residual contributions to
  std::vector<MooseVariable *> _master_save_in_residual_variables;

  /// Whether there are slave residual aux variables
  bool _has_slave_residuals_saved_in;

  /// The aux variables to save the slave contributions to
  std::vector<MooseVariable *> _slave_save_in_residual_variables;

  /** MultiMooseEnum specifying whether jacobian save-in
   * aux variables correspond to master or slave side
   */
  MultiMooseEnum _diag_save_in_var_side;

  /** The names of the aux variables that will be used to save-in jacobians
   * (includes both master and slave variable names)
   */
  std::vector<AuxVariableName> _diag_save_in_strings;

  /// Whether there are master jacobian aux variables
  bool _has_master_jacobians_saved_in;

  /// The aux variables to save the diagonal Jacobian contributions of the master variables to
  std::vector<MooseVariable *> _master_save_in_jacobian_variables;

  /// Whether there are slave jacobian aux variables
  bool _has_slave_jacobians_saved_in;

  /// The aux variables to save the diagonal Jacobian contributions of the slave variables to
  std::vector<MooseVariable *> _slave_save_in_jacobian_variables;

  /// Mutex that prevents multiple threads from saving into the residual aux_var at the same time
  static Threads::spin_mutex _resid_vars_mutex;

  /// Mutex that prevents multiple threads from saving into the jacobian aux_var at the same time
  static Threads::spin_mutex _jacoby_vars_mutex;
};

#endif
