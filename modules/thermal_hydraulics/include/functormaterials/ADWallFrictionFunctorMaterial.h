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
 * Converts Darcy friction factor function into material property
 */
class ADWallFrictionFunctorMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();

  ADWallFrictionFunctorMaterial(const InputParameters & parameters);

protected:
  const Moose::Functor<ADReal> & _functor;

  /// Name of the friction factor functor
  const MooseFunctorName _f_D_name;
};
