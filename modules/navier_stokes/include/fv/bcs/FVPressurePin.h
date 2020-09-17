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

template <typename>
class MooseVariableFV;
class FEProblemBase;
class SystemBase;
class MooseMesh;

/**
 * Constrain a single dof for the pressure variable
 */
class FVPressurePin final : public NodalBCBase, public MooseVariableInterface<Real>
{
public:
  static InputParameters validParams();

  FVPressurePin(const InputParameters & parameters);

  void computeResidual() override;
  void computeJacobian() override;
  void computeOffDiagJacobian(unsigned int jvar) override;

  const MooseVariableFieldBase & variable() const override { return _var; }

  bool checkNodalVar() const override { return false; }

private:
  /**
   * @return the index for the pressure variable on an element connected to the node
   */
  dof_id_type getDofIndex();

  /**
   * emit a boundary parameter error message telling the user we should only execute on a single
   * node
   */
  void boundaryParamError();

  /// the pressure variable
  MooseVariableFV<Real> & _var;

  /// Reference to FEProblemBase
  FEProblemBase & _fe_problem;

  /// Reference to SystemBase
  SystemBase & _sys;

  /// Mesh this BC is defined on
  MooseMesh & _mesh;

  /// The Dirichlet value
  const Real & _value;

  /// The only node_id that this object should be executed on. If another proc owns the constrained
  /// node, then \p _node_id should be \p DofObject::invalid_id
  dof_id_type _node_id;
};
