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

class ArrayHFEMDirichletTestBC : public ArrayLowerDIntegratedBC
{
public:
  static InputParameters validParams();

  ArrayHFEMDirichletTestBC(const InputParameters & parameters);

protected:
  virtual void computeQpResidual(RealEigenVector & residual) override;
  virtual void computeLowerDQpResidual(RealEigenVector & residual) override;
  virtual RealEigenVector computeLowerDQpJacobian(Moose::ConstraintJacobianType type) override;
  virtual RealEigenMatrix computeLowerDQpOffDiagJacobian(Moose::ConstraintJacobianType type,
                                                         const MooseVariableFEBase & jvar) override;

  // return a matrix for linear transformation of a vector
  RealEigenMatrix transform();

  /// Boundary values
  const RealEigenVector _value;
  /// flag for PJFNK
  const bool & _for_pjfnk;
};
