//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalBCBase.h"
#include "MooseVariableInterface.h"

/**
 * Base class for deriving any automatic differentiation boundary condition of a integrated type
 */
template <typename T>
class ADNodalBCTempl : public NodalBCBase, public MooseVariableInterface<T>
{
public:
  static InputParameters validParams();

  ADNodalBCTempl(const InputParameters & parameters);

  const MooseVariableFE<T> & variable() const override { return _var; }

protected:
  /**
   * Compute this NodalBC's contribution to the residual at the current quadrature point
   */
  virtual typename Moose::ADType<T>::type computeQpResidual() = 0;

  /// The variable that this NodalBC operates on
  MooseVariableFE<T> & _var;

  /// current node being processed
  const Node * const & _current_node;

  /// Pseudo-"quadrature point" index (Always zero for the current node)
  const unsigned int _qp = 0;

  /// Value of the unknown variable this BC is acting on
  const typename Moose::ADType<T>::type & _u;

  const std::array<bool, 3> _set_components;

private:
  void computeResidual() override final;
  void computeJacobian() override final;
  void computeResidualAndJacobian() override;
  void computeOffDiagJacobian(unsigned int jvar) override final;
  void computeOffDiagJacobianScalar(unsigned int jvar) override final;

  /**
   * process the residual into the global data structures
   */
  template <typename ADResidual>
  void processResidual(const ADResidual & residual, const std::vector<dof_id_type> & dof_indices);

  /**
   * process the Jacobian into the global data structures
   */
  template <typename ADResidual>
  void processJacobian(const ADResidual & residual, const std::vector<dof_id_type> & dof_indices);

  /// A reference to the undisplaced assembly in order to ensure data gets correctly incorporated
  /// into the global residual/Jacobian
  Assembly & _undisplaced_assembly;
};

using ADNodalBC = ADNodalBCTempl<Real>;
using ADVectorNodalBC = ADNodalBCTempl<RealVectorValue>;
