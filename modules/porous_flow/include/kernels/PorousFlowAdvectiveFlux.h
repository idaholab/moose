/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWADVECTIVEFLUX_H
#define POROUSFLOWADVECTIVEFLUX_H

#include "PorousFlowDarcyBase.h"

class PorousFlowAdvectiveFlux;

template<>
InputParameters validParams<PorousFlowAdvectiveFlux>();

/**
 * Convective flux of component k in fluid phase alpha.
 * A fully-updwinded version is implemented, where the mobility
 * of the upstream nodes is used.
 */
class PorousFlowAdvectiveFlux : public PorousFlowDarcyBase
{
public:
  PorousFlowAdvectiveFlux(const InputParameters & parameters);

protected:
  /** The mobility of the fluid.
   * This is mass_fraction * fluid_density * relative_permeability / fluid_viscosity
   * @param nodenum The node-number to evaluate the mobility for
   * @param phase the fluid phase number
   */
  virtual Real mobility(unsigned nodenum, unsigned phase);

  /** The derivative of mobility with respect to PorousFlow variable pvar
   * @param nodenum The node-number to evaluate the mobility for
   * @param phase the fluid phase number
   * @param pvar the PorousFlow variable pvar
   */
  virtual Real dmobility(unsigned nodenum, unsigned phase, unsigned pvar);

  /// Mass fraction of each component in each phase
  const MaterialProperty<std::vector<std::vector<Real> > > & _mass_fractions;

  /// Derivative of the mass fraction of each component in each phase wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<std::vector<Real> > > > & _dmass_fractions_dvar;

  /// Relative permeability of each phase
  const MaterialProperty<std::vector<Real> > & _relative_permeability;

  /// Derivative of relative permeability of each phase wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real> > > & _drelative_permeability_dvar;

  /// Index of the fluid component that this kernel acts on
  const unsigned int _fluid_component;
};

#endif // POROUSFLOWADVECTIVEFLUX_H
