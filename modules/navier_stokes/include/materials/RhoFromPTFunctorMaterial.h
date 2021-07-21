//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

class SinglePhaseFluidProperties;

/**
 * Computes the speed of sound from other Navier-Stokes material properties
 */
class RhoFromPTFunctorMaterial : public Material
{
public:
  RhoFromPTFunctorMaterial(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  /// pressure
  const MooseVariableFVReal & _pressure;

  /// temperature
  const MooseVariableFVReal & _temperature;

  /// fluid properties user object
  const SinglePhaseFluidProperties & _fluid;

  /// the density to be computed
  FunctorMaterialProperty<ADReal> & _rho;
};
