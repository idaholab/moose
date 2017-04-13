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

// MOOSE includes
#include "Constraint.h"
#include "NeighborCoupleableMooseVariableDependencyIntermediateInterface.h"

// Forward Declarations
class NodeFaceConstraint;

// libMesh forward declarations
namespace libMesh
{
template <typename T>
class SparseMatrix;
}

template <>
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
class NodeFaceConstraint : public Constraint,
                           public NeighborCoupleableMooseVariableDependencyIntermediateInterface
{
public:
  NodeFaceConstraint(const InputParameters & parameters);
  virtual ~NodeFaceConstraint();

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
   * Computes d-residual / d-jvar...
   */
  virtual void computeOffDiagJacobian(unsigned int jvar);

  /**
   * Gets the indices for all dofs connected to the constraint
   */
  virtual void getConnectedDofIndices(unsigned int var_num);

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
  virtual bool overwriteSlaveResidual();

  /**
   * Whether or not the slave's Jacobian row should be overwritten.
   *
   * When this returns true the slave's Jacobian row as computed by the constraint will _replace_
   * the residual previously at that node for that variable.
   */
  virtual bool overwriteSlaveJacobian() { return overwriteSlaveResidual(); };

  /**
   * The variable on the Master side of the domain.
   */
  virtual MooseVariable & masterVariable() { return _master_var; }

  // TODO: Make this protected or add an accessor
  // Do the same for all the other public members
  SparseMatrix<Number> * _jacobian;

protected:
  /**
   * Compute the value the slave node should have at the beginning of a timestep.
   */
  virtual Real computeQpSlaveValue() = 0;

  /**
   * This is the virtual that derived classes should override for computing the residual on
   * neighboring element.
   */
  virtual Real computeQpResidual(Moose::ConstraintType type) = 0;

  /**
   * This is the virtual that derived classes should override for computing the Jacobian on
   * neighboring element.
   */
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type) = 0;

  /**
   * This is the virtual that derived classes should override for computing the off-diag Jacobian.
   */
  virtual Real computeQpOffDiagJacobian(Moose::ConstraintJacobianType /*type*/,
                                        unsigned int /*jvar*/)
  {
    return 0;
  }

  /// coupling interface:
  virtual const VariableValue & coupledSlaveValue(const std::string & var_name,
                                                  unsigned int comp = 0)
  {
    return coupledValue(var_name, comp);
  }
  virtual const VariableValue & coupledSlaveValueOld(const std::string & var_name,
                                                     unsigned int comp = 0)
  {
    return coupledValueOld(var_name, comp);
  }
  virtual const VariableValue & coupledSlaveValueOlder(const std::string & var_name,
                                                       unsigned int comp = 0)
  {
    return coupledValueOlder(var_name, comp);
  }

  virtual const VariableGradient & coupledSlaveGradient(const std::string & var_name,
                                                        unsigned int comp = 0)
  {
    return coupledGradient(var_name, comp);
  }
  virtual const VariableGradient & coupledSlaveGradientOld(const std::string & var_name,
                                                           unsigned int comp = 0)
  {
    return coupledGradientOld(var_name, comp);
  }
  virtual const VariableGradient & coupledSlaveGradientOlder(const std::string & var_name,
                                                             unsigned int comp = 0)
  {
    return coupledGradientOlder(var_name, comp);
  }

  virtual const VariableSecond & coupledSlaveSecond(const std::string & var_name,
                                                    unsigned int comp = 0)
  {
    return coupledSecond(var_name, comp);
  }

  virtual const VariableValue & coupledMasterValue(const std::string & var_name,
                                                   unsigned int comp = 0)
  {
    return coupledNeighborValue(var_name, comp);
  }
  virtual const VariableValue & coupledMasterValueOld(const std::string & var_name,
                                                      unsigned int comp = 0)
  {
    return coupledNeighborValueOld(var_name, comp);
  }
  virtual const VariableValue & coupledMasterValueOlder(const std::string & var_name,
                                                        unsigned int comp = 0)
  {
    return coupledNeighborValueOlder(var_name, comp);
  }

  virtual const VariableGradient & coupledMasterGradient(const std::string & var_name,
                                                         unsigned int comp = 0)
  {
    return coupledNeighborGradient(var_name, comp);
  }
  virtual const VariableGradient & coupledMasterGradientOld(const std::string & var_name,
                                                            unsigned int comp = 0)
  {
    return coupledNeighborGradientOld(var_name, comp);
  }
  virtual const VariableGradient & coupledMasterGradientOlder(const std::string & var_name,
                                                              unsigned int comp = 0)
  {
    return coupledNeighborGradientOlder(var_name, comp);
  }

  virtual const VariableSecond & coupledMasterSecond(const std::string & var_name,
                                                     unsigned int comp = 0)
  {
    return coupledNeighborSecond(var_name, comp);
  }

  /// Boundary ID for the slave surface
  unsigned int _slave;
  /// Boundary ID for the master surface
  unsigned int _master;

  const MooseArray<Point> & _master_q_point;
  QBase *& _master_qrule;

public:
  PenetrationLocator & _penetration_locator;

protected:
  /// current node being processed
  const Node *& _current_node;
  const Elem *& _current_master;

  /// Value of the unknown variable this BC is action on
  const VariableValue & _u_slave;
  /// Shape function on the slave side.  This will always
  VariablePhiValue _phi_slave;
  /// Shape function on the slave side.  This will always only have one entry and that entry will always be "1"
  VariableTestValue _test_slave;

  /// Master side variable
  MooseVariable & _master_var;

  /// Number for the master variable
  unsigned int _master_var_num;

  /// Side shape function.
  const VariablePhiValue & _phi_master;
  /// Gradient of side shape function
  const VariablePhiGradient & _grad_phi_master;

  /// Side test function
  const VariableTestValue & _test_master;
  /// Gradient of side shape function
  const VariableTestGradient & _grad_test_master;

  /// Holds the current solution at the current quadrature point
  const VariableValue & _u_master;
  /// Holds the current solution gradient at the current quadrature point
  const VariableGradient & _grad_u_master;

  /// DOF map
  const DofMap & _dof_map;

  const std::map<dof_id_type, std::vector<dof_id_type>> & _node_to_elem_map;

  /**
   * Whether or not the slave's residual should be overwritten.
   *
   * When this is true the slave's residual as computed by the constraint will _replace_
   * the residual previously at that node for that variable.
   */
  bool _overwrite_slave_residual;

public:
  std::vector<dof_id_type> _connected_dof_indices;

  DenseMatrix<Number> _Kne;
  DenseMatrix<Number> _Kee;
};

#endif
