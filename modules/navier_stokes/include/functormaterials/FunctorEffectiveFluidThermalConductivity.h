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
 * This is a base class material to calculate the effective thermal
 * conductivity of the fluid phase. Unlike for the solid effective thermal
 * conductivity, a factor of porosity is removed such that all parameters in the
 * fluid effective thermal conductivity calculation can be expressed as porosity
 * multiplying some parameter as $\kappa_f=\epsilon K$, where this material
 * actually computes $K$. This is performed such that the correct spatial
 * derivative of $\kappa_f$ can be performed, since the porosity is in general a
 * function of space, and spatial derivatives of materials cannot be computed.
 */
class FunctorEffectiveFluidThermalConductivity : public FunctorMaterial
{
public:
  FunctorEffectiveFluidThermalConductivity(const InputParameters & parameters);

  static InputParameters validParams();
};
