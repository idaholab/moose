//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ArrayLowerDIntegratedBC.h"

class ArrayHFEMDirichletBC : public ArrayLowerDIntegratedBC
{
public:
  static InputParameters validParams();

  ArrayHFEMDirichletBC(const InputParameters & parameters);

protected:
  virtual void computeQpResidual(RealEigenVector & residual) override;
  virtual void computeLowerDQpResidual(RealEigenVector & residual) override;
  virtual RealEigenVector computeLowerDQpJacobian(Moose::ConstraintJacobianType type) override;
  virtual RealEigenMatrix computeLowerDQpOffDiagJacobian(Moose::ConstraintJacobianType,
                                                         const MooseVariableFEBase & jvar) override;

  /// Boundary values
  const RealEigenVector _value;
  /// Variable coupled in
  const ArrayMooseVariable * const _uhat_var;
  /// Holds the coupled solution at the current quadrature point on the face.
  const ArrayVariableValue * const _uhat;
};
