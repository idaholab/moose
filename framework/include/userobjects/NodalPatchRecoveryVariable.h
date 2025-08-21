//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "NodalPatchRecoveryBase.h"

/**
 * Patch recovery from a coupled variable
 */
class NodalPatchRecoveryVariable : public NodalPatchRecoveryBase
{
public:
  static InputParameters validParams();
  NodalPatchRecoveryVariable(const InputParameters & params);

  /// Returns the variable name
  const VariableName & variableName() const;

protected:
  /// Returns the recovered value at the quadrature point (_qp)
  Real computeValue() override;

private:
  /// Variable value
  const VariableValue & _v;

  /// Variable name
  const VariableName _name;
};
