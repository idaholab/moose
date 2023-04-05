//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"

class ADMomentumViscousRZ : public ADKernel
{
public:
  static InputParameters validParams();

  ADMomentumViscousRZ(const InputParameters & parameters);

  virtual ~ADMomentumViscousRZ() {}

protected:
  virtual ADReal computeQpResidual() override;

  const ADMaterialProperty<Real> & _mu;
  const Moose::CoordinateSystemType & _coord_sys;
  const unsigned int _rz_radial_coord;
  const unsigned short _component;
};
