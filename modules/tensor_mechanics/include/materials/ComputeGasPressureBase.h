/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEGASPRESSUREBASE_H
#define COMPUTEGASPRESSUREBASE_H

#include "ComputeExtraStressBase.h"

class ComputeGasPressureBase;

template <>
InputParameters validParams<ComputeGasPressureBase>();

/**
 * Sets up an isotropic extra Stress with a trace that equals the pressure of
 * an ideal gas with the given density.
 * \f$ PV=Nk_BT \f$
 */
class ComputeGasPressureBase : public ComputeExtraStressBase
{
public:
  ComputeGasPressureBase(const InputParameters & parameters);

protected:
  /// concentration
  const VariableValue & _c;

  /// temperature
  const VariableValue & _T;

  /// lattice site volume
  const Real _omega;

  // Pressure unit conversion factor
  const Real _pressure_unit_conversion;

  // Boltzmann constant
  const Real _kB;
};

#endif // COMPUTEGASPRESSUREBASE_H
