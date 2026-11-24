//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MultiPointConstraint.h"

/**
 * Ties the value of a variable at each node of a boundary to its value at the coincident node of
 * another boundary, with one exact constraint per node.
 */
class TestMultiPointConstraint : public MultiPointConstraint
{
public:
  static InputParameters validParams();

  TestMultiPointConstraint(const InputParameters & parameters);

  virtual void addExactConstraints(libMesh::DofMap & dof_map) const override;

  virtual const MooseVariableBase & variable() const override { return *_variables[0]; }

protected:
  /// The variable whose degrees of freedom this constraint ties together
  const std::vector<const MooseVariable *> _variables;

  /// The boundary whose nodes hold the dependent degrees of freedom
  const BoundaryName & _secondary_boundary;

  /// The boundary whose nodes hold the degrees of freedom that the dependent ones are tied to
  const BoundaryName & _primary_boundary;
};
