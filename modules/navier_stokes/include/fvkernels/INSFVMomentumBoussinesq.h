//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVElementalKernel.h"
#include "INSFVMomentumResidualObject.h"

/**
 * Imposes a Boussinesq force on the momentum equation. Useful for modeling natural convection
 * within an incompressible formulation of the Navier-Stokes equations
 */
class INSFVMomentumBoussinesq : public FVElementalKernel, public INSFVMomentumResidualObject
{
public:
  static InputParameters validParams();
  INSFVMomentumBoussinesq(const InputParameters & params);

  void gatherRCData(const Elem &) override {}
  void gatherRCData(const FaceInfo &) override {}

protected:
  ADReal computeQpResidual() override;

  /// Fluid temperature
  const Moose::Functor<ADReal> & _temperature;
  /// The gravity vector
  const RealVectorValue _gravity;
  /// The thermal expansion coefficient
  const Moose::Functor<ADReal> & _alpha;
  /// Reference temperature at which the value of _rho was measured
  const Real _ref_temperature;
  /// the density
  const Moose::Functor<ADReal> & _rho;
};
