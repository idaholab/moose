//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RadiativeHeatFluxBCBase.h"

/**
 * Radiative heat transfer boundary condition for a plate heat structure
 */
class RadiativeHeatFluxBC : public RadiativeHeatFluxBCBase
{
public:
  RadiativeHeatFluxBC(const InputParameters & parameters);

protected:
  virtual Real coefficient() const override;

  /// Emissivity of the boundary
  const Real _eps_boundary;
  /// View factor function
  const Function & _view_factor_fn;

  /// Post-processor by which to scale boundary condition
  const PostprocessorValue & _scale_pp;

public:
  static InputParameters validParams();
};
