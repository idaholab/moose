//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVAdvectionDiffusionFunctorRobinBCBase.h"

class GrayLambertSurfaceRadiationBase;

/**
 * Applies a Gray-Lambert surface-to-surface radiative heat-flux boundary condition
 * to a linear finite-volume advection-diffusion equation.
 */
class LinearFVGrayLambertBC : public LinearFVAdvectionDiffusionFunctorRobinBCBase
{
public:
  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  LinearFVGrayLambertBC(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual Real getAlpha(Moose::FaceArg face, Moose::StateArg state) const override;
  virtual Real getBeta(Moose::FaceArg face, Moose::StateArg state) const override;
  virtual Real getGamma(Moose::FaceArg face, Moose::StateArg state) const override;

  /// Temperature functor used to reconstruct the local surface emission
  const Moose::Functor<Real> & _temperature_radiation;
  /// Diffusion coefficient multiplying the outward normal temperature gradient
  const Moose::Functor<Real> & _coeff_diffusion;
  /// User object providing surface emissivity, irradiation, and net radiative heat flux
  const GrayLambertSurfaceRadiationBase & _glsr_uo;
  /// Whether to reconstruct the local emitted heat flux using the boundary-face temperature
  bool _reconstruct_emission;
};
