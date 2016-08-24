/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWCONVECTIVEFLUX_H
#define POROUSFLOWCONVECTIVEFLUX_H

#include "PorousFlowDarcyBase.h"

class PorousFlowConvectiveFlux;

template<>
InputParameters validParams<PorousFlowConvectiveFlux>();

/**
 * Convective flux of component k in fluid phase alpha.
 * A fully-updwinded version is implemented, where the mobility
 * of the upstream nodes is used.
 */
class PorousFlowConvectiveFlux : public PorousFlowDarcyBase
{
public:
  PorousFlowConvectiveFlux(const InputParameters & parameters);

protected:
  /** The mobility of the fluid.
   * This is enthalpy * fluid_density * relative_permeability / fluid_viscosity
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

  /// Enthalpy of each phase
  const MaterialProperty<std::vector<Real> > & _enthalpy;

  /// Derivative of the enthalpy wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real> > > & _denthalpy_dvar;

  /// Relative permeability of each phase
  const MaterialProperty<std::vector<Real> > & _relative_permeability;

  /// Derivative of relative permeability of each phase wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real> > > & _drelative_permeability_dvar;
};

#endif // POROUSFLOWCONVECTIVEFLUX_H
