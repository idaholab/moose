//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVDiffusion.h"

/// INSFVTurbulentDiffusion implements a standard diffusion term for a turbulent problem:
///
///     - strong form: \nabla \cdot k \nabla u / coef
///
///     - weak form: \int_{A} k \nabla u / coef \cdot \vec{n} dA
///
/// It uses/requests a material property named "coeff" for k. An average of
/// the elem and neighbor k-values (which should be face-values) is used to
/// compute k on the face. Cross-diffusion correction factors are currently not
/// implemented for the "grad_u*n" term.
/// The specialty of this kernel is that it takes into account the wall treatment of the variable with respect to turbulence.
class INSFVTurbulentDiffusion : public FVDiffusion
{
public:
  static InputParameters validParams();
  virtual void initialSetup() override;
  INSFVTurbulentDiffusion(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override final;
  using FVDiffusion::computeResidual;
  void computeResidual(const FaceInfo & fi) override;
  using FVDiffusion::computeJacobian;
  void computeJacobian(const FaceInfo & fi) override;

  const Moose::Functor<ADReal> & _scaling_coef;

  /// Wall boundaries
  const std::vector<BoundaryName> & _wall_boundary_names;

  /// Maps for wall treatment
  std::map<const Elem *, bool> _wall_bounded;

  /// Whether a Newton's method is being used (and we need to preserve the sparsity pattern in edge cases)
  const bool _preserve_sparsity_pattern;
};
