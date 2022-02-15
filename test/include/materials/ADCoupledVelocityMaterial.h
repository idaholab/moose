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
  const Moose::Functor<ADReal> & _vel_x;
  const Moose::Functor<ADReal> * const _vel_y;
  const Moose::Functor<ADReal> * const _vel_z;
  const Moose::Functor<ADReal> & _rho;
};
