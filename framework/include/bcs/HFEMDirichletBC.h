//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LowerDIntegratedBC.h"

class HFEMDirichletBC : public LowerDIntegratedBC
{
public:
  static InputParameters validParams();

  HFEMDirichletBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeLowerDQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeLowerDQpJacobian(Moose::ConstraintJacobianType type) override;
  virtual Real computeLowerDQpOffDiagJacobian(Moose::ConstraintJacobianType,
                                              const MooseVariableFEBase & jvar) override;

  /// Boundary values
  const Real _value;
  /// Variable coupled in
  const MooseVariable * const _uhat_var;
  /// Holds the coupled solution at the current quadrature point on the face.
  const VariableValue * const _uhat;
};
