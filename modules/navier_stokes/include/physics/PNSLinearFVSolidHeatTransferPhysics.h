//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PNSFVSolidHeatTransferPhysics.h"

/**
 * Creates all the objects needed to solve the porous media solid energy equation with the linear
 * finite volume discretization, for use with the segregated solvers
 */
class PNSLinearFVSolidHeatTransferPhysics final : public PNSFVSolidHeatTransferPhysics
{
public:
  static InputParameters validParams();

  PNSLinearFVSolidHeatTransferPhysics(const InputParameters & parameters);

private:
  virtual void addSolverVariables() override;
  virtual void addInitialConditions() override;
  virtual void addFVKernels() override;
  virtual void addFVBCs() override;

  /// Return the solid fraction of the volume, (1 - porosity), as a functor name
  MooseFunctorName addSolidFractionFunctor();

  /// Whether to use the correction term for non-orthogonality in the conduction term
  const bool _non_orthogonal_correction;
};
