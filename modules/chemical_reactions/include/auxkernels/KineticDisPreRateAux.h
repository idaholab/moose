/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef KINETICDISPRERATEAUX_H
#define KINETICDISPRERATEAUX_H

#include "AuxKernel.h"

// Forward Declarations
class KineticDisPreRateAux;

template <>
InputParameters validParams<KineticDisPreRateAux>();

/**
 * Define the AuxKernel for the kinetic mineral species kinetic rate
 * according to transient state theory rate law.
 */
class KineticDisPreRateAux : public AuxKernel
{
public:
  KineticDisPreRateAux(const InputParameters & parameters);

  virtual ~KineticDisPreRateAux() {}

protected:
  virtual Real computeValue() override;

  /// Equilibrium constant at reference temperature
  Real _log_k;

  /// Specific reactive surface area, m^2/L solution
  Real _r_area;

  /// Reference kinetic rate constant
  const Real _ref_kconst;

  /// Activation energy
  Real _e_act;

  /// Gas constant, 8.314 J/mol/K
  const Real _gas_const;

  /// Reference temperature
  Real _ref_temp;

  /// Actual system temperature
  Real _sys_temp;

  /// Stoichiometric coefficients for involved primary species
  std::vector<Real> _sto_v;

  /// Coupled primary species concentrations
  std::vector<const VariableValue *> _vals;
};

#endif // KINETICDISPRERATEAUX_H
