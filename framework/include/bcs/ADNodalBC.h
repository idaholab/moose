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

private:
  void computeResidual() override final;
  void computeJacobian() override final;
  void computeOffDiagJacobian(unsigned int jvar) override final;
  void computeOffDiagJacobianScalar(unsigned int jvar) override final;

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

  const std::vector<bool> _set_components;
};

using ADNodalBC = ADNodalBCTempl<Real>;
using ADVectorNodalBC = ADNodalBCTempl<RealVectorValue>;
