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

class NSFVMassFluxAdvectionBC : public FVFluxBC
{
public:
  static InputParameters validParams();
  NSFVMassFluxAdvectionBC(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  /// The mass flux on the element
  const ADMaterialProperty<RealVectorValue> & _mass_flux_elem;

  /// The mass flux on the neighbor
  const ADMaterialProperty<RealVectorValue> & _mass_flux_neighbor;

  const ADMaterialProperty<Real> & _adv_elem;
  const ADMaterialProperty<Real> & _adv_neighbor;

  /// The interpolation method to use for the flux quantity
  Moose::FV::InterpMethod _flux_interp_method;
};
