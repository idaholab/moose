//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVMatAdvectionOutflowBC.h"

class PNSFVMomentumOneOverGammaMatAdvectionOutflowBC : public FVMatAdvectionOutflowBC
{
public:
  static InputParameters validParams();
  PNSFVMomentumOneOverGammaMatAdvectionOutflowBC(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  const MaterialProperty<Real> & _eps_elem;
  const MaterialProperty<Real> & _eps_neighbor;

  /// The interpolation method to use for the advected quantity
  Moose::FV::InterpMethod _one_over_porosity_interp_method;
};
