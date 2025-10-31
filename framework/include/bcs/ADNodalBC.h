//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseVariableInterface.h"
#include "NodalBCBase.h"
#include "ADDirichletBCBase.h"
#include "ADFunctorInterface.h"

/**
 * Base class for deriving any automatic differentiation boundary condition of a integrated type
 */
template <typename T, typename Base>
class ADNodalBCTempl : public Base, public MooseVariableInterface<T>, public ADFunctorInterface
{
public:
  static InputParameters validParams();

  ADNodalBCTempl(const InputParameters & parameters);

  const MooseVariableFE<T> & variable() const override { return _var; }

  bool shouldSetComp(unsigned short i) const { return _set_components[i]; }

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

  std::vector<bool> _set_components;

  using Base::_fe_problem;
  using Base::_subproblem;
  using Base::_sys;
  using Base::_tid;
  using Base::addJacobian;
  using Base::addMooseVariableDependency;
  using Base::setResidual;

private:
  void computeResidual() override final;
  void computeJacobian() override final;
  void computeResidualAndJacobian() override;
  void computeOffDiagJacobian(unsigned int jvar) override final;
  void computeOffDiagJacobianScalar(unsigned int jvar) override final;

  /**
   * process the residual into the global data structures
   */
  void addResidual(const T & residual, const std::vector<dof_id_type> & dof_indices);

  /**
   * process the Jacobian into the global data structures
   */
  template <typename ADResidual>
  void addJacobian(const ADResidual & residual, const std::vector<dof_id_type> & dof_indices);

  /// A reference to the undisplaced assembly in order to ensure data gets correctly incorporated
  /// into the global residual/Jacobian
  Assembly & _undisplaced_assembly;
};

template <>
InputParameters ADNodalBCTempl<RealVectorValue, NodalBCBase>::validParams();

template <>
InputParameters ADNodalBCTempl<RealVectorValue, ADDirichletBCBase>::validParams();

using ADNodalBC = ADNodalBCTempl<Real, NodalBCBase>;
using ADVectorNodalBC = ADNodalBCTempl<RealVectorValue, NodalBCBase>;
using ADArrayNodalBC = ADNodalBCTempl<RealEigenVector, NodalBCBase>;
