//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ArrayDGLowerDKernel.h"

class ArrayHFEMDiffusionTest : public ArrayDGLowerDKernel
{
public:
  static InputParameters validParams();

  ArrayHFEMDiffusionTest(const InputParameters & parameters);

protected:
  virtual void computeQpResidual(Moose::DGResidualType type, RealEigenVector & residual) override;
  virtual void computeLowerDQpResidual(RealEigenVector & residual) override;
  virtual RealEigenVector computeLowerDQpJacobian(Moose::ConstraintJacobianType) override;
  virtual RealEigenMatrix computeLowerDQpOffDiagJacobian(Moose::ConstraintJacobianType type,
                                                         const MooseVariableFEBase & jvar) override;

  // return a matrix for linear transformation of a vector
  RealEigenMatrix transform();

  const bool & _for_pjfnk;
};
