//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HDGKernel.h"

class MassMatrixHDG : public HDGKernel
{
public:
  static InputParameters validParams();

  MassMatrixHDG(const InputParameters & parameters);

  virtual void computeResidual() override {}
  virtual void computeJacobian() override {}
  virtual void computeOffDiagJacobian(unsigned int) override {}
  virtual void computeResidualOnSide() override {}
  virtual void computeJacobianOnSide() override;

protected:
  const MooseArray<std::vector<Real>> & _face_phi;
  const Real _density;
  DenseMatrix<Number> _mass;
};
