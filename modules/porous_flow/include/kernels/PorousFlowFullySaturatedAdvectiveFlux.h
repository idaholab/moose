//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowDarcyBase.h"

/**
 * Convective flux of component k in a single-phase fluid
 * A fully-updwinded version is implemented, where the mobility
 * of the upstream nodes is used.
 */
class PorousFlowFullySaturatedAdvectiveFlux : public PorousFlowDarcyBase
{
public:
  static InputParameters validParams();

  PorousFlowFullySaturatedAdvectiveFlux(const InputParameters & parameters);

protected:
  virtual Real mobility(unsigned nodenum, unsigned phase) const override;
  virtual Real dmobility(unsigned nodenum, unsigned phase, unsigned pvar) const override;

  /// Mass fraction of each component in each phase
  const MaterialProperty<std::vector<std::vector<Real>>> & _mass_fractions;

  /// Derivative of the mass fraction of each component in each phase wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<std::vector<Real>>>> & _dmass_fractions_dvar;

  /// Index of the fluid component that this kernel acts on
  const unsigned int _fluid_component;

  /// Whether the flux is multiplied by density (so it will be a mass flux) or not (it will be a volume flux)
  const bool _multiply_by_density;
};
