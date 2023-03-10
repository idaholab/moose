//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeGasPressureBase.h"

/**
 * Sets up an isotropic extra Stress with a trace that equals the pressure of
 * an ideal gas with the given density.
 * \f$ PV=Nk_BT \f$
 */
class ComputeVanDerWaalsGasPressure : public ComputeGasPressureBase
{
public:
  static InputParameters validParams();

  ComputeVanDerWaalsGasPressure(const InputParameters & parameters);

protected:
  virtual void computeQpExtraStress();

  /// Van der Waals coefficient a in [eV*Ang^3/molecule] (default units)
  const Real _a;

  /// Van der Waals molecular volume in [Ang^3/molecule] (default units)
  const Real _b;
};
