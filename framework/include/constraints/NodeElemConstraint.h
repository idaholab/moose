//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "NodeElemConstraintBase.h"

/**
 * A NodeElemConstraint is used when you need to create constraints between
 * a secondary node and a primary element. It works by allowing you to modify the
 * residual and jacobian entries on the secondary node and the primary element.
 */
class NodeElemConstraint : public NodeElemConstraintBase
{
public:
  static InputParameters validParams();

  NodeElemConstraint(const InputParameters & parameters);
  virtual ~NodeElemConstraint();

  /// Computes the residual Nodal residual.
  virtual void computeResidual() override;

  /// Computes the jacobian for the current element.
  virtual void computeJacobian() override;

  /// Computes d-residual / d-jvar...
  virtual void computeOffDiagJacobian(unsigned int jvar) override;

  /// Gets the indices for all dofs connected to the constraint
  virtual void getConnectedDofIndices(unsigned int var_num);

protected:
  /// prepare the _secondary_to_primary_map
  virtual void prepareSecondaryToPrimaryMap() = 0;

  /// This is the virtual that derived classes should override for computing the residual.
  virtual Real computeQpResidual(Moose::ConstraintType type) = 0;

  /// This is the virtual that derived classes should override for computing the Jacobian.
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType /*type*/)
  {
    return 0;
  } // fixme mooseERRROR This is so AD can build

  /// This is the virtual that derived classes should override for computing the off-diag Jacobian.
  virtual Real computeQpOffDiagJacobian(Moose::ConstraintJacobianType /*type*/,
                                        unsigned int /*jvar*/)
  {
    return 0;
  }

  // coupling interface:
  virtual const VariableValue & coupledSecondaryValue(const std::string & var_name,
                                                      unsigned int comp = 0)
  {
    return coupledValue(var_name, comp);
  }
  virtual const VariableValue & coupledSecondaryValueOld(const std::string & var_name,
                                                         unsigned int comp = 0)
  {
    return coupledValueOld(var_name, comp);
  }
  virtual const VariableValue & coupledSecondaryValueOlder(const std::string & var_name,
                                                           unsigned int comp = 0)
  {
    return coupledValueOlder(var_name, comp);
  }

  virtual const VariableGradient & coupledSecondaryGradient(const std::string & var_name,
                                                            unsigned int comp = 0)
  {
    return coupledGradient(var_name, comp);
  }
  virtual const VariableGradient & coupledSecondaryGradientOld(const std::string & var_name,
                                                               unsigned int comp = 0)
  {
    return coupledGradientOld(var_name, comp);
  }
  virtual const VariableGradient & coupledSecondaryGradientOlder(const std::string & var_name,
                                                                 unsigned int comp = 0)
  {
    return coupledGradientOlder(var_name, comp);
  }

  virtual const VariableSecond & coupledSecondarySecond(const std::string & var_name,
                                                        unsigned int comp = 0)
  {
    return coupledSecond(var_name, comp);
  }

  virtual const VariableValue & coupledPrimaryValue(const std::string & var_name,
                                                    unsigned int comp = 0)
  {
    return coupledNeighborValue(var_name, comp);
  }
  virtual const VariableValue & coupledPrimaryValueOld(const std::string & var_name,
                                                       unsigned int comp = 0)
  {
    return coupledNeighborValueOld(var_name, comp);
  }
  virtual const VariableValue & coupledPrimaryValueOlder(const std::string & var_name,
                                                         unsigned int comp = 0)
  {
    return coupledNeighborValueOlder(var_name, comp);
  }

  virtual const VariableGradient & coupledPrimaryGradient(const std::string & var_name,
                                                          unsigned int comp = 0)
  {
    return coupledNeighborGradient(var_name, comp);
  }
  virtual const VariableGradient & coupledPrimaryGradientOld(const std::string & var_name,
                                                             unsigned int comp = 0)
  {
    return coupledNeighborGradientOld(var_name, comp);
  }
  virtual const VariableGradient & coupledPrimaryGradientOlder(const std::string & var_name,
                                                               unsigned int comp = 0)
  {
    return coupledNeighborGradientOlder(var_name, comp);
  }

  virtual const VariableSecond & coupledPrimarySecond(const std::string & var_name,
                                                      unsigned int comp = 0)
  {
    return coupledNeighborSecond(var_name, comp);
  }

  /// secondary block id
  unsigned short _secondary;
  /// primary block id
  unsigned short _primary;

  const MooseArray<Point> & _primary_q_point;
  const QBase * const & _primary_qrule;

  /// current node being processed
  const Node * const & _current_node;
  const Elem * const & _current_elem;

  /// Value of the unknown variable on the secondary node
  const VariableValue & _u_secondary;
  /// old solution
  const VariableValue & _u_secondary_old;
  /// Shape function on the secondary side.
  VariablePhiValue _phi_secondary;
  /// Shape function on the secondary side.  This will always only have one entry and that entry will always be "1"
  VariableTestValue _test_secondary;

  /// Number for the primary variable
  unsigned int _primary_var_num;

  /// Side shape function.
  const VariablePhiValue & _phi_primary;
  /// Gradient of side shape function
  const VariablePhiGradient & _grad_phi_primary;

  /// Side test function
  const VariableTestValue & _test_primary;
  /// Gradient of side shape function
  const VariableTestGradient & _grad_test_primary;

  /// Holds the current solution at the current quadrature point
  const VariableValue & _u_primary;
  /// Holds the old solution at the current quadrature point
  const VariableValue & _u_primary_old;
  /// Holds the current solution gradient at the current quadrature point
  const VariableGradient & _grad_u_primary;

  /// DOF map
  const DofMap & _dof_map;

  const std::map<dof_id_type, std::vector<dof_id_type>> & _node_to_elem_map;

  /// maps secondary node ids to primary element ids
  std::map<dof_id_type, dof_id_type> _secondary_to_primary_map;
};
