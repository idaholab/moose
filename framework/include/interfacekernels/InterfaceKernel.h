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

#include "DGKernel.h"
#include "TwoMaterialPropertyInterface.h"

// Forward Declarations
class InterfaceKernel;

template <>
InputParameters validParams<InterfaceKernel>();

/**
 * InterfaceKernel is responsible for interfacing physics across subdomains
 */

class InterfaceKernel : public DGKernelBase, public TwoMaterialPropertyInterface
{
public:
  InterfaceKernel(const InputParameters & parameters);

  /**
   * The neighbor variable number that this kernel operates on
   */
  const MooseVariable & neighborVariable() const;

  virtual void computeElemNeighResidual(Moose::DGResidualType type);
  virtual void computeElemNeighJacobian(Moose::DGJacobianType type);
  virtual void computeJacobian();
  virtual void computeOffDiagElemNeighJacobian(Moose::DGJacobianType type, unsigned int jvar);
  virtual void computeElementOffDiagJacobian(unsigned int jvar);
  virtual void computeNeighborOffDiagJacobian(unsigned int jvar);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) = 0;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) = 0;
  virtual Real computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar);

  MooseVariable & _neighbor_var;

  const VariableValue & _neighbor_value;
  const VariableGradient & _grad_neighbor_value;

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
};

#endif
