//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADIntegratedBC.h"

/**
 * Radiative heat transfer boundary condition for a plate heat structure
 */
class ADRadiativeHeatFluxBC : public ADIntegratedBC
{
public:
  ADRadiativeHeatFluxBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// Ambient temperature
  const Moose::Functor<ADReal> & _T_ambient;
  /// Emissivity
  const Moose::Functor<ADReal> & _emissivity;
  /// View factor
  const Moose::Functor<ADReal> & _view_factor;

  /// Post-processor by which to scale boundary condition
  const PostprocessorValue & _scale_pp;
  /// Functor by which to scale the boundary condition
  const Moose::Functor<ADReal> & _scale;

  /// Stefan-Boltzmann constant
  const Real _sigma;

public:
  static InputParameters validParams();
};
