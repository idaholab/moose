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

#ifndef NODEFACECONSTRAINT_H
#define NODEFACECONSTRAINT_H

//MOOSE includes
#include "Moose.h"
#include "DiracKernelData.h"
#include "DiracKernelInfo.h"
#include "MooseObject.h"
#include "Coupleable.h"
#include "FunctionInterface.h"
#include "MaterialPropertyInterface.h"
#include "TransientInterface.h"
#include "GeometricSearchInterface.h"
#include "MooseVariable.h"
#include "SubProblemInterface.h"
#include "MooseMesh.h"

//libMesh includes
#include "libmesh_common.h"
#include "elem.h"
#include "point.h"
#include "sparse_matrix.h"
//#include "ValidParams.h"

//Forward Declarations
class AsmBlock;

class NodeFaceConstraint;

template<>
InputParameters validParams<NodeFaceConstraint>();

/**
 * A NodeFaceConstraint is used when you need to create constraints between
 * two surfaces in a mesh.  It works by allowing you to modify the residual
 * and jacobian entries on "this" side (the node side, also referred to as
 * the slave side) and the "other" side (the face side, also referred to as
 * the master side)
 *
 * This is common for contact algorithms and other constraints.
 */
class NodeFaceConstraint :
  public MooseObject,
  public Coupleable,
  public FunctionInterface,
  public TransientInterface,
  private GeometricSearchInterface
{
public:
  NodeFaceConstraint(const std::string & name, InputParameters parameters);
  virtual ~NodeFaceConstraint(){}

  /**
   * Compute the value the slave node should have at the beginning of a timestep.
   */
  void computeSlaveValue(NumericVector<Number> & current_solution);

  /**
   * Computes the residual Nodal residual.
   */
  virtual void computeResidual();

  /**
   * Computes the jacobian for the current element.
   */
  virtual void computeJacobian();

  /**
   * The variable number that this object operates on.
   */
  MooseVariable & variable() { return _var; }

  SubProblemInterface & subProblem() { return _subproblem; }

  /**
   * Compute the value the slave node should have at the beginning of a timestep.
   */
  virtual Real computeQpSlaveValue() = 0;

  /**
   * This is the virtual that derived classes should override for computing the residual on neighboring element.
   */
  virtual Real computeQpResidual(Moose::ConstraintType type) = 0;

  /**
   * This is the virtual that derived classes should override for computing the Jacobian on neighboring element.
   */
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type) = 0;

  /**
   * Whether or not this constraint should be applied.
   *
   * Get's called once per slave node.
   */
  virtual bool shouldApply() { return true; }

  /**
   * Whether or not the slave's residual should be overwritten.
   *
   * When this returns true the slave's residual as computed by the constraint will _replace_
   * the residual previously at that node for that variable.
   */
  bool overwriteSlaveResidual() { return _overwrite_slave_residual; }

  SparseMatrix<Number> * _jacobian;
protected:
  unsigned int _slave;  /// Boundary ID for the slave surface
  unsigned int _master; /// Boundary ID for the master surface

  Problem & _problem;
  SubProblemInterface & _subproblem;
  SystemBase & _sys;

  THREAD_ID _tid;

  AsmBlock & _asmb;
  AssemblyData & _asm_data;
  MooseVariable & _var;
  MooseMesh & _mesh;
  unsigned int _dim;

  unsigned int _i, _j;
  unsigned int _qp;

  const std::vector< Point > & _master_q_point;
  QBase * & _master_qrule;

public:
  PenetrationLocator & _penetration_locator;
protected:

  const Node * & _current_node;                                       ///< current node being processed
  const Elem * & _current_master;

  VariableValue & _u_slave;                                           ///< Value of the unknown variable this BC is action on
  std::vector<std::vector<Real> > _phi_slave;                   /// Shape function on the slave side.  This will always
  std::vector<std::vector<Real> > _test_slave;                  /// Shape function on the slave side.  This will always only have one entry and that entry will always be "1"

  const std::vector<std::vector<Real> > & _phi_master;                /// Side shape function.
  const std::vector<std::vector<RealGradient> > & _grad_phi_master;   /// Gradient of side shape function
  const std::vector<std::vector<RealTensor> > & _second_phi_master;   /// Second derivative of side shape function

  const std::vector<std::vector<Real> > & _test_master;               /// Side test function
  const std::vector<std::vector<RealGradient> > & _grad_test_master;  /// Gradient of side shape function
  const std::vector<std::vector<RealTensor> > & _second_test_master;  /// Second derivative of side shape function

  VariableValue & _u_master;                                          /// Holds the current solution at the current quadrature point
  VariableValue & _u_old_master;                                      /// Holds the previous solution at the current quadrature point.
  VariableValue & _u_older_master;                                    /// Holds the t-2 solution at the current quadrature point.

  VariableGradient & _grad_u_master;                                  /// Holds the current solution gradient at the current quadrature point
  VariableGradient & _grad_u_old_master;                              /// Holds the previous solution gradient at the current quadrature point
  VariableGradient & _grad_u_older_master;                            /// Holds the t-2 solution gradient at the current quadrature point

  VariableSecond  & _second_u_master;                                 /// Holds the current solution second derivative at the current quadrature point

  const DofMap & _dof_map;                                            /// DOF map

  std::vector<std::vector<unsigned int> > & _node_to_elem_map;

  /**
   * Whether or not the slave's residual should be overwritten.
   *
   * When this is true the slave's residual as computed by the constraint will _replace_
   * the residual previously at that node for that variable.
   */
  bool _overwrite_slave_residual;

public:
  std::vector<unsigned int> _connected_dof_indices;

  DenseMatrix<Number> _Kne;
  DenseMatrix<Number> _Kee;
};

#endif
