//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowFullySaturatedDarcyBase.h"

/**
 * Darcy advective flux for a fully-saturated,
 * single-phase, multi-component fluid.
 * No upwinding or relative-permeability is used.
 */
class PorousFlowFullySaturatedDarcyFlow : public PorousFlowFullySaturatedDarcyBase
{
public:
  static InputParameters validParams();

  PorousFlowFullySaturatedDarcyFlow(const InputParameters & parameters);

protected:
  /**
   * The mobility of the fluid = mass_fraction * density / viscosity
   */
  virtual Real mobility() const override;

  /**
   * The derivative of the mobility with respect to the PorousFlow variable pvar
   * @param pvar Take the derivative with respect to this PorousFlow variable
   */
  virtual Real dmobility(unsigned pvar) const override;

  /// mass fraction of the components in the phase
  const MaterialProperty<std::vector<std::vector<Real>>> & _mfrac;

  /// Derivative of mass fraction wrt wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<std::vector<Real>>>> & _dmfrac_dvar;

  /// The fluid component for this Kernel
  const unsigned int _fluid_component;
};
