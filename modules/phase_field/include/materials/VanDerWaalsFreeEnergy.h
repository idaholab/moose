//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GasFreeEnergyBase.h"

// Forward Declarations

/**
 * Material class that provides the free energy of a Van der Waals gas with the
 * expression builder and uses automatic differentiation to get the derivatives.
 */
class VanDerWaalsFreeEnergy : public GasFreeEnergyBase
{
public:
  static InputParameters validParams();

  VanDerWaalsFreeEnergy(const InputParameters & parameters);

protected:
  /**
   * Van der Waals coefficient a in [eV*Ang^3] (default units)
   *
   * Data from "Physics", M. Alonso, E.J. Finn (ISBN-13: 978-0201565188):
   * He:  0.0593 eV*Ang^3 = 0.0095e-48 Pa*m^6 * 6.241506363e+18 eV/(Pa*m^3) * 1e30 Ang^3/m^3
   * Ar:  2.3275 eV*Ang^3
   * Xe:  7.3138 eV*Ang^3
   * Hg: 14.1133 eV*Ang^3
   */
  const Real _a;

  /* Van der Waals molecular volume in [Ang^3] (default units)
   *
   * Data from "Physics", M. Alonso, E.J. Finn (ISBN-13: 978-0201565188):
   * He: 39.36 Ang^3 = 3.3936e-29 m^3 * 1e30 Ang^3/m^3
   * Ar: 53.45 Ang^3
   * Xe: 84.77 Ang^3
   * Hg: 28.16 Ang^3
   */
  const Real _b;

  /// Taylor expansion threshold for the logarithm in the free energy
  const Real _log_tol;
};
