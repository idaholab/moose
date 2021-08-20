//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"

class ADCoupledVelocityMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();

  ADCoupledVelocityMaterial(const InputParameters & parameters);

protected:
  FunctorMaterialProperty<ADRealVectorValue> & _velocity;
  FunctorMaterialProperty<ADReal> & _rho_u;
  FunctorMaterialProperty<ADReal> & _rho_v;
  FunctorMaterialProperty<ADReal> & _rho_w;

  const FunctorInterface<ADReal> & _vel_x;
  const FunctorInterface<ADReal> * const _vel_y;
  const FunctorInterface<ADReal> * const _vel_z;
  const FunctorInterface<ADReal> & _rho;
};
