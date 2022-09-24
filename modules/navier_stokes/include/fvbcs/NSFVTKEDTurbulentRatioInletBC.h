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

/**
 * Neumann boundary (== fixed inflow) condition for finite volume scheme
 */
class NSFVTKEDTurbulentRatioInletBC : public FVFluxBC
{
public:
  NSFVTKEDTurbulentRatioInletBC(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual ADReal computeQpResidual() override;

  /// x-velocity
  const Moose::Functor<ADReal> & _u_var;
  /// y-velocity
  const Moose::Functor<ADReal> * _v_var;
  /// z-velocity 
  const Moose::Functor<ADReal> * _w_var;

  /// Density
  const Moose::Functor<ADReal> & _rho;

  /// Dynamic vicosity
  const Moose::Functor<ADReal> & _mu;

  /// C_mu closure constant
  const Moose::Functor<ADReal> & _C_mu;

  /// Mixing Length
  const Moose::Functor<ADReal> & _turbulent_ratio;

  /// Trubulent intensity
  const Real _intensity;
};
