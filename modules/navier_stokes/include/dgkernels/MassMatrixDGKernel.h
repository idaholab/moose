//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DGKernel.h"

/**
 * This class can be used to compute a mass matrix for facet unknowns, e.g. for variables that live
 * only on element faces (these are \p SIDE_HIERARCHIC finite element family variables in libMesh).
 * This object is only executed on internal faces so in general this object should be paired with
 * MassMatrixIntegratedBC for external faces. This class cannot be used if performing static
 * condensation because static condensation relies on computing residuals and Jacobians on a strict
 * per-element basis whereas DGKernels simultaneously add residuals and Jacobians to both elements
 * on either side of the face they're currently operating on
 */
class MassMatrixDGKernel : public DGKernel
{
public:
  static InputParameters validParams();

  MassMatrixDGKernel(const InputParameters & parameters);

  virtual void computeResidual() override {}

protected:
  virtual Real computeQpResidual(Moose::DGResidualType) override;

  virtual Real computeQpJacobian(const Moose::DGJacobianType type) override;

  const Real _density;
};
