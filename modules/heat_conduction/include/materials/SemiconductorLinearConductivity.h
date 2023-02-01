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
#include "DerivativeMaterialInterface.h"

// Forward Declarations

/**
 * Calculates resistivity and electrical conductivity as a function of temperature.
 * For semiconductor, Steinhart-Hart equation describes the temperature dependence
 * of the resistivity: 1/T = A + B ln (\rho) + C (ln (\rho))^3
 *
 * For linear relationship ONLY => C = 0
 *      1/T = A -B ln(\sigma)
 *   \rho: electrical resistivity in ohm-m = 1/(\sigma)
 *   \sigma: electrical conductivity in 1/(ohm-m)
 */
class SemiconductorLinearConductivity : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  SemiconductorLinearConductivity(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

private:
  const Real _sh_coeff_A;
  const Real _sh_coeff_B;
  const VariableValue & _T;

  const std::string _base_name;
  MaterialProperty<Real> & _electric_conductivity;
  MaterialProperty<Real> * _delectric_conductivity_dT;
};
