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

class ADMortarConstraint : public MortarConstraintBase
{
public:
  static InputParameters validParams();

  ADMortarConstraint(const InputParameters & parameters);

protected:
  /**
   * compute the residual at the quadrature points
   */
  virtual ADReal computeQpResidual(Moose::MortarType mortar_type) = 0;

  using MortarConstraintBase::computeResidual;
  /**
   * compute the residual for the specified element type
   */
  void computeResidual(Moose::MortarType mortar_type) override;

  using MortarConstraintBase::computeJacobian;
  /**
   * compute the residual for the specified element type
   */
  void computeJacobian(Moose::MortarType mortar_type) override;

  /**
   * Get rid of AD derivative entries by dof index
   */
  void trimDerivative(dof_id_type remove_derivative_index, ADReal & dual_number);

  /**
   * Get rid of interior node variable's derivatives
   */
  template <typename Variables, typename DualNumbers>
  void
  trimInteriorNodeDerivatives(const std::map<unsigned int, unsigned int> & primary_ip_lowerd_map,
                              const Variables & moose_var,
                              DualNumbers & ad_vars,
                              const bool is_secondary);

  void computeResidualAndJacobian() override;

private:
  /// A dummy object useful for constructing _lambda when not using Lagrange multipliers
  const ADVariableValue _lambda_dummy;

protected:
  /// The LM solution
  const ADVariableValue & _lambda;

  /// The primal solution on the secondary side
  const ADVariableValue & _u_secondary;

  /// The primal solution on the primary side
  const ADVariableValue & _u_primary;

  /// The primal solution gradient on the secondary side
  const ADVariableGradient & _grad_u_secondary;

  /// The primal solution gradient on the primary side
  const ADVariableGradient & _grad_u_primary;
};

template <typename Variables, typename DualNumbers>
void
ADMortarConstraint::trimInteriorNodeDerivatives(
    const std::map<unsigned int, unsigned int> & domain_ip_lowerd_map,
    const Variables & moose_vars,
    DualNumbers & dual_numbers,
    const bool is_secondary)
{
  // Remove interior node variable's derivatives from AD objects.
  for (const auto dof_index :
       (is_secondary ? make_range(_test_secondary.size()) : make_range(_test_primary.size())))
    if (!domain_ip_lowerd_map.count(dof_index))
    {
      for (const auto * const moose_var : moose_vars)
      {
        // It's valid for a user to pass a container that represents a LIBMESH_DIM vector of
        // component variables for which one or two of the variables may be null depending on the
        // mesh dimension in the simulation
        if (!moose_var)
          continue;

        mooseAssert(moose_var->isNodal(),
                    "Trimming of interior node's derivatives is only supported for Lagrange "
                    "elements in mortar constraints");

        const auto remove_derivative_index = is_secondary
                                                 ? moose_var->dofIndices()[dof_index]
                                                 : moose_var->dofIndicesNeighbor()[dof_index];
        for (auto & dual_number : dual_numbers)
          trimDerivative(remove_derivative_index, dual_number);
      }
    }
}
