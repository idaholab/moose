/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEVANDERWAALSGASPRESSURE_H
#define COMPUTEVANDERWAALSGASPRESSURE_H

#include "ComputeGasPressureBase.h"

class ComputeVanDerWaalsGasPressure;

template <>
InputParameters validParams<ComputeVanDerWaalsGasPressure>();

/**
 * Sets up an isotropic extra Stress with a trace that equals the pressure of
 * an ideal gas with the given density.
 * \f$ PV=Nk_BT \f$
 */
class ComputeVanDerWaalsGasPressure : public ComputeGasPressureBase
{
public:
  ComputeVanDerWaalsGasPressure(const InputParameters & parameters);

protected:
  virtual void computeQpExtraStress();

  /// Van der Waals coefficient a in [eV*Ang^3/molecule] (default units)
  const Real _a;

  /// Van der Waals molecular volume in [Ang^3/molecule] (default units)
  const Real _b;
};

#endif // COMPUTEVANDERWAALSGASPRESSURE_H
