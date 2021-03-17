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

class PCNSFVImplicitMomentumPressureBC : public FVFluxBC
{
public:
  static InputParameters validParams();
  PCNSFVImplicitMomentumPressureBC(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  const ADMaterialProperty<Real> & _pressure;
  const MaterialProperty<Real> & _eps;
  const unsigned int _index;
};
