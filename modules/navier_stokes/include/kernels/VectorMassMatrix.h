//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "VectorKernel.h"
#include "MooseFunctor.h"

/**
 * Computes a finite element mass matrix meant for use in preconditioning schemes which require one
 */
class VectorMassMatrix : public VectorKernel
{
public:
  static InputParameters validParams();

  VectorMassMatrix(const InputParameters & parameters);

  virtual void computeResidual() override {}

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

private:
  // The density for scaling the mass matrix
  const Real _density;
};
