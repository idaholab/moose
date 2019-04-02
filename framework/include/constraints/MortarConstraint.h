//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MortarConstraintBase.h"

#include "libmesh/quadrature_gauss.h"

// Forward Declarations
template <ComputeStage>
class MortarConstraint;
class FEProblemBase;

declareADValidParams(MortarConstraint);

template <ComputeStage compute_stage>
class MortarConstraint : public MortarConstraintBase
{
public:
  MortarConstraint(const InputParameters & parameters);

  virtual void computeResidual() override;

  virtual void computeJacobian() override;

  virtual SubdomainID masterSubdomain() const override { return _master_subdomain_id; }

  MooseVariable * variable() override { return _var; }

  virtual const VariableValue & value() override;

  virtual const VariableGradient & gradient() override;

  virtual const VariableValue & coupledValue(const std::string & var_name,
                                             unsigned int comp) override;

  virtual const VariableGradient & coupledGradient(const std::string & var_name,
                                                   unsigned int comp) override;

  /**
   * Returns variables needed for values
   */
  virtual set::set<MooseVariable *> & neededVariablesForValues()
  {
    return _variables_for_which_values_requested;
  }

  /**
   * Returns variables needed for gradients
   */
  virtual set::set<MooseVariable *> & neededVariablesForGradients()
  {
    return _variables_for_which_gradients_requested;
  }

protected:
  /**
   * compute the Lagrange Multipler equation at the quadrature points
   */
  virtual ADResidual computeQpResidual() { return 0; }

  /**
   * compute the primal equation at the quadrature points. `type` is either `Moose::Slave` or
   * `Moose::Master`
   */
  virtual ADResidual computeQpResidualSide(Moose::ConstraintType /*type*/) { return 0; }

  /// Whether the current mortar segment projects onto a face on the master side
  bool _has_master;

  /// Reference to the finite element problem
  FEProblemBase & _fe_problem;

  /// Reference to the SystemBase that this object will contribute its residual and Jacobian to
  SystemBase & _sys;

  /// Boundary ID for the slave surface
  const BoundaryID _slave_id;

  /// Boundary ID for the master surface
  const BoundaryID _master_id;

  /// Subdomain ID for the slave surface
  const SubdomainID _slave_subdomain_id;

  /// Subdomain ID for the master surface
  const SubdomainID _master_subdomain_id;

  /// The mortar mesh generator object
  AutomaticMortarGeneration & _amg;

  /// Pointer to the lagrange multipler variable. nullptr if none
  MooseVariable * _var;

  /// Reference to the slave variable
  MooseVariable & _slave_var;

  /// Reference to the master variable
  MooseVariable & _master_var;

  /// The primal var number
  const unsigned int _primal_var_number;

  /// The lagrange multiplier var number
  const unsigned int _lambda_var_number;

  /// Whether to compute primal residuals
  const bool _compute_primal_residuals;

  /// Whether to compute lagrange multiplier residuals
  const bool _compute_lm_residuals;

  /// The dof map
  const DofMap & _dof_map;

  /// The dimensionality of the volume elements
  const unsigned int _interior_dimension;

  /// The dimensionality of the mortar segment mesh, e.g. the dimension of face
  /// elements. Equivalent to _interior_dimension - 1
  const unsigned int _msm_dimension;

  /// The lower-dimensional (mortar face) FE object for the primal variable
  std::unique_ptr<FEBase> _fe_msm_primal;

  /// FE object for the lagrange multiplier (note that the LM only exists on the
  /// lower dimesional mesh elements, e.g. the mortar interface boundary)
  std::unique_ptr<FEBase> _fe_msm_lambda;

  /// The interior slave (volume) FE object for the primal variable
  std::unique_ptr<FEBase> _fe_slave_interior_primal;

  /// The interior master (volume) FE object for the primal variable
  std::unique_ptr<FEBase> _fe_master_interior_primal;

  /// the normals along the slave face
  const std::vector<Point> & _normals;

  /// The mortar segment quadrature rule
  QGauss _qrule_msm;

  /// The Jacobian times weights for the mortar segment FE
  const std::vector<Real> & _JxW_msm;

  /// A dummy object useful for constructing _test when not using Lagrange multipliers
  const std::vector<std::vector<Real>> _dummy;

  /// The shape functions corresponding to the lagrange multiplier variable
  const std::vector<std::vector<Real>> & _test;

  /// The shape functions corresponding to the slave interior primal variable
  const std::vector<std::vector<Real>> & _test_slave;

  /// The shape functions corresponding to the master interior primal variable
  const std::vector<std::vector<Real>> & _test_master;

  /// The shape function gradients corresponding to the slave interior primal variable
  const std::vector<std::vector<RealVectorValue>> & _grad_test_slave;

  /// The shape function gradients corresponding to the master interior primal variable
  const std::vector<std::vector<RealVectorValue>> & _grad_test_master;

  /// The locations of the quadrature points on the interior slave elements
  const std::vector<Point> & _phys_points_slave;

  /// The locations of the quadrature points on the interior master elements
  const std::vector<Point> & _phys_points_master;

  /// The dof indices for the Lagrange Multiplier variable that only lives on the mortar interface
  std::vector<dof_id_type> _dof_indices_lambda;

  /// The dof indices for the primal variable on the slave interior element
  std::vector<dof_id_type> _dof_indices_slave_interior_primal;

  /// The dof indices for the primal variable on the master interior element
  std::vector<dof_id_type> _dof_indices_master_interior_primal;

  /// Quadrature point index
  unsigned int _qp;

  /// The LM solution
  MooseArray<ADReal> _lambda;

  /// The primal solution on the slave side
  MooseArray<ADReal> _u_slave;

  /// The primal solution on the master side
  MooseArray<ADReal> _u_master;

  /// The primal solution gradient on the slave side
  MooseArray<VectorValue<ADReal>> _grad_u_slave;

  /// The primal solution gradient on the master side
  MooseArray<VectorValue<ADReal>> _grad_u_master;

  /// The offset for LM dofs for derivative vector access
  const unsigned int _lm_offset;

  /// The offset for slave primal dofs for derivative vector access
  unsigned int _slave_primal_offset;

  /// The offset for master primal dofs for derivative vector access
  unsigned int _master_primal_offset;

  /// Whether we need to calculate the primal gradients
  bool _need_primal_gradient;

  /// Whether this constraint is going to be used to enforce a periodic condition. This has the
  /// effect of changing the normals vector for projection from outward to inward facing
  bool _periodic;

private:
  /**
   * Loop over the mortar mesh and compute either the residual or Jacobian depending on
   * compute_stage
   */
  void loopOverMortarMesh();

  /**
   * compute the residual on the current element
   */
  void computeElementResidual();

  /**
   * compute the jacobian on the current element
   */
  void computeElementJacobian();

  /**
   * compute the variable solutions on the current element
   */
  void computeSolutions();

private:
  /// A set of variables for which solution values have been requested
  std::set<MooseVariable *> _variables_for_which_values_requested;

  /// A set of variables for which solution gradients have been requested
  std::set<MooseVariable *> _variables_for_which_gradients_requested;

protected:
  usingCoupleableMembers;
};

#define usingMortarConstraintMembers                                                               \
  using MortarConstraint<compute_stage>::_phys_points_slave;                                       \
  using MortarConstraint<compute_stage>::_phys_points_master;                                      \
  using MortarConstraint<compute_stage>::_lambda;                                                  \
  using MortarConstraint<compute_stage>::_u_slave;                                                 \
  using MortarConstraint<compute_stage>::_u_master;                                                \
  using MortarConstraint<compute_stage>::_qp;                                                      \
  using MortarConstraint<compute_stage>::_has_master;                                              \
  using MortarConstraint<compute_stage>::_i;                                                       \
  using MortarConstraint<compute_stage>::_test;                                                    \
  using MortarConstraint<compute_stage>::_test_slave;                                              \
  using MortarConstraint<compute_stage>::_test_master;                                             \
  using MortarConstraint<compute_stage>::_need_primal_gradient;                                    \
  using MortarConstraint<compute_stage>::_grad_u_slave;                                            \
  using MortarConstraint<compute_stage>::_grad_u_master;                                           \
  using MortarConstraint<compute_stage>::_grad_test_slave;                                         \
  using MortarConstraint<compute_stage>::_grad_test_master;                                        \
  using MortarConstraint<compute_stage>::_normals

#endif /* MORTARCONSTRAINT_H */
