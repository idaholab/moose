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
#include "ZeroInterface.h"
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
                        public ZeroInterface,
                        public MeshChangedInterface,
                        public TwoMaterialPropertyInterface
{
public:
  InterfaceKernel(const InputParameters & parameters);

  /*
   * The master variable that this interface kernel operates on
   */
  MooseVariable & variable() const;

  /**
   * The neighbor variable number that this interface kernel operates on
   */
  const MooseVariable & neighborVariable() const;

  /**
   * Return a reference to the subproblem.
   */
  SubProblem & subProblem();

  virtual void computeElemNeighResidual(Moose::DGResidualType type);
  virtual void computeElemNeighJacobian(Moose::DGJacobianType type);
  virtual void computeOffDiagElemNeighJacobian(Moose::DGJacobianType type, unsigned int jvar);

  virtual void computeElementOffDiagJacobian(unsigned int jvar);
  virtual void computeNeighborOffDiagJacobian(unsigned int jvar);

  /**
   * Computes the residual for the current side.
   */
  virtual void computeResidual();

  /**
   * Computes the jacobian for the current side.
   */
  virtual void computeJacobian();

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) = 0;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) = 0;
  virtual Real computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar);
  /// The volume of the current neighbor
  const Real & getNeighborElemVolume();

  SubProblem & _subproblem;
  SystemBase & _sys;

  THREAD_ID _tid;

  Assembly & _assembly;
  MooseVariable & _var;
  MooseMesh & _mesh;
  //  unsigned int _dim;

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

  static Threads::spin_mutex _resid_vars_mutex;
  static Threads::spin_mutex _jacoby_vars_mutex;
};

#endif
