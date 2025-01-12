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

class SinglePhaseFluidProperties;

/**
 * This is the material class used to compute enthalpy for the incompressible/weakly-compressible
 * finite-volume implementation of the Navier-Stokes equations
 */
class INSFVEnthalpyFunctorMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();

  INSFVEnthalpyFunctorMaterial(const InputParameters & parameters);

protected:
  /// whether we can use a constant cp as a shortcut to compute enthalpy
  bool _assumed_constant_cp;

  /// A fluid properties user object to compute enthalpy
  const SinglePhaseFluidProperties * _fp;

  /// density
  const Moose::Functor<ADReal> & _rho;

  /// the temperature
  const Moose::Functor<ADReal> & _temperature;

  /// the pressure
  const Moose::Functor<ADReal> * _pressure;

  /// the specific heat capacity
  const Moose::Functor<ADReal> & _cp;

  /// the specific enthalpy
  const Moose::Functor<ADReal> * _h;
};
