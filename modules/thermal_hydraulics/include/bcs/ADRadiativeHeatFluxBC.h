//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
class ADRadiativeHeatFluxBC : public ADRadiativeHeatFluxBCBase
{
public:
  ADRadiativeHeatFluxBC(const InputParameters & parameters);

protected:
  virtual ADReal coefficient() const override;

  /// View factor function
  const Function & _view_factor_fn;

  /// Post-processor by which to scale boundary condition
  const PostprocessorValue & _scale_pp;

public:
  static InputParameters validParams();
};
