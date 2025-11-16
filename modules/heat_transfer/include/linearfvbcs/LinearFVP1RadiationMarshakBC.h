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

/**
 * Class implementing a Marshak boundary condition for P1 radiation model in
 * linear finite volume variables. This is only applicable for advection-diffusion problems.
 */
class LinearFVP1RadiationMarshakBC : public LinearFVAdvectionDiffusionFunctorRobinBCBase
{
public:
  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  LinearFVP1RadiationMarshakBC(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual Real getAlpha(Moose::FaceArg face, Moose::StateArg state) const override;
  virtual Real getBeta(Moose::FaceArg face, Moose::StateArg state) const override;
  virtual Real getGamma(Moose::FaceArg face, Moose::StateArg state) const override;

  /// Functor giving the alpha coefficient (multiplying normal gradient)
  const Moose::Functor<Real> & _temperature_radiation;
  /// Functor giving the beta coefficient (multiplying value)
  const Moose::Functor<Real> & _coeff_diffusion;
  /// Functor giving the gamma coefficient (on right hand side, treated explicitly)
  const Moose::Functor<Real> & _eps_boundary;
};
