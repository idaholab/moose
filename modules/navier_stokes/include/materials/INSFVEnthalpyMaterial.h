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
 * This is the material class used to compute enthalpy for the incompressible/weakly-compressible
 * finite-volume implementation of the Navier-Stokes equations
 */
class INSFVEnthalpyMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();

  INSFVEnthalpyMaterial(const InputParameters & parameters);

protected:
  /// density
  const Moose::Functor<ADReal> & _rho;

  /// the temperature
  const Moose::Functor<ADReal> & _temperature;

  /// the specific heat capacity
  const Moose::Functor<ADReal> & _cp;
};
