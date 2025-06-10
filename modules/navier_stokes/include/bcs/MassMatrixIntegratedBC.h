//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"

/**
 * This class adds the exterior boundary mass for facet unknowns. It should be used in conjunction
 * with a mass matrix object operating on interior faces (such as MassMatrixDGKernel if not
 * performing static condensation) which computes the facet mass for interior faces.
 */
class MassMatrixIntegratedBC : public IntegratedBC
{
public:
  static InputParameters validParams();

  MassMatrixIntegratedBC(const InputParameters & parameters);

  virtual void computeResidual() override {}

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  const Real _density;
};
