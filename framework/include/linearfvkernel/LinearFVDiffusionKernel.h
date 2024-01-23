//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVFluxKernel.h"

class LinearFVDiffusionKernel : public LinearFVFluxKernel
{
public:
  static InputParameters validParams();
  LinearFVDiffusionKernel(const InputParameters & params);

  virtual Real computeElemMatrixContribution() override;

  virtual Real computeNeighborMatrixContribution() override;

  virtual Real computeElemRightHandSideContribution() override;

  virtual Real computeNeighborRightHandSideContribution() override;

  virtual Real computeBoundaryMatrixContribution(const LinearFVBoundaryCondition * bc) override;

  virtual Real computeBoundaryRHSContribution(const LinearFVBoundaryCondition * bc) override;

protected:
  Real getCorrectionContribution();

  Real computeFluxMatrixContribution();

  Real computeFluxRHSContribution();

  /// The functor for the diffusion coefficient
  const Moose::Functor<Real> & _diffusion_coeff;

  /// Switch to enable/disable nonorthogonal correction
  const bool _use_nonorthogonal_correction;

  Moose::FaceArg _current_face_arg;

  Real _current_face_diffusivity;

  Real _current_delta;

  Real _flux_matrix_contribution;
  Real _flux_rhs_contribution;
};
