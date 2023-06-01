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

/**
 * Material class used to compute the interstitial velocity norm for the incompressible and weakly
 * compressible primitive superficial finite-volume implementation of porous media equations
 */
class PINSFVSpeedFunctorMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();

  PINSFVSpeedFunctorMaterial(const InputParameters & parameters);

protected:
  /// Porosity
  const Moose::Functor<ADReal> & _eps;

  ///@{
  /// Superficial velocity
  const Moose::Functor<ADReal> & _superficial_vel_x;
  const Moose::Functor<ADReal> & _superficial_vel_y;
  const Moose::Functor<ADReal> & _superficial_vel_z;
  ///@}
};
