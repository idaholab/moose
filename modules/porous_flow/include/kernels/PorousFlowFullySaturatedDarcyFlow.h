/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWFULLYSATURATEDDARCYFLOW_H
#define POROUSFLOWFULLYSATURATEDDARCYFLOW_H

#include "PorousFlowFullySaturatedDarcyBase.h"

class PorousFlowFullySaturatedDarcyFlow;

template <>
InputParameters validParams<PorousFlowFullySaturatedDarcyFlow>();

/**
 * Darcy advective flux for a fully-saturated,
 * single-phase, multi-component fluid.
 * No upwinding or relative-permeability is used.
 */
class PorousFlowFullySaturatedDarcyFlow : public PorousFlowFullySaturatedDarcyBase
{
public:
  PorousFlowFullySaturatedDarcyFlow(const InputParameters & parameters);

protected:
  /**
   * The mobility of the fluid = mass_fraction * density / viscosity
   */
  virtual Real mobility() const override;

  /**
   * The derivative of the mobility with respect to the porous-flow variable pvar
   * @param pvar Take the derivative with respect to this porous-flow variable
   */
  virtual Real dmobility(unsigned pvar) const override;

  /// mass fraction of the components in the phase
  const MaterialProperty<std::vector<std::vector<Real>>> & _mfrac;

  /// Derivative of mass fraction wrt wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<std::vector<Real>>>> & _dmfrac_dvar;

  /// The fluid component for this Kernel
  const unsigned int _fluid_component;
};

#endif // POROUSFLOWFULLYSATURATEDDARCYFLOW_H
