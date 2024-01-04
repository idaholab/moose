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
#include "MathFVUtils.h"
#include "INSFVMomentumResidualObject.h"
#include "INSFVVelocityVariable.h"

/**
 * Computes the source and sink terms for the turbulent kinetic energy dissipation rate.
 */
class INSFVTV2SourceSink : public FVElementalKernel
{
public:
  static InputParameters validParams();

  INSFVTV2SourceSink(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

protected:
  /// Turbulent kinetic energy
  const Moose::Functor<ADReal> & _k;

  /// Turbulent kinetic energy dissipation rate
  const Moose::Functor<ADReal> & _epsilon;

  /// Relaxation function
  const Moose::Functor<ADReal> & _f;

  /// n fitting factor
  const Real _n;
};
