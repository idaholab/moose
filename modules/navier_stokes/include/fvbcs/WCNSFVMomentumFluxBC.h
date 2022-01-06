//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxBC.h"
#include "INSFVFlowBC.h"

/**
 * Flux boundary conditions for the weakly compressible boundary equation
 */
class WCNSFVMomentumFluxBC : public FVFluxBC, public INSFVFlowBC
{
public:
  static InputParameters validParams();
  WCNSFVMomentumFluxBC(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  /// Scaling factor
  const Real _scaling_factor;

  /// Postprocessor with the inlet velocity
  const PostprocessorValue * const _velocity_pp;

  /// Postprocessor with the inlet mass flow rate
  const PostprocessorValue * const _mdot_pp;

  /// Postprocessor with the inlet area
  const PostprocessorValue * const _area_pp;

  /// Fluid density functor
  const Moose::Functor<ADReal> * const _rho;
};
