//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADIntegratedBC.h"

class GaussianWeldEnergyFluxBC : public ADIntegratedBC
{
public:
  static InputParameters validParams();

  GaussianWeldEnergyFluxBC(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  const Real _reff;
  const Real _F0;
  const Real _R;
  const Function & _x_beam_coord;
  const Function & _y_beam_coord;
  const Function & _z_beam_coord;
};
