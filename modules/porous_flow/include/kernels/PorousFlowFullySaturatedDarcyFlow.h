//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowFullySaturatedDarcyBase.h"

/**
 * Darcy advective flux for a fully-saturated, single-phase, multi-component fluid.
 * Extends DarcyBase by multiplying the mobility by the component mass fraction.
 * No upwinding or relative-permeability effects are included.
 */
template <bool is_ad>
class PorousFlowFullySaturatedDarcyFlowTempl : public PorousFlowFullySaturatedDarcyBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowFullySaturatedDarcyFlowTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> mobility() const override;
  virtual Real dmobility(unsigned int pvar) const override;

  /// Mass fraction of each component in each phase
  const GenericMaterialProperty<std::vector<std::vector<Real>>, is_ad> & _mfrac;

  /// d(mass fraction)/d(PorousFlow variable) — null for AD path
  const MaterialProperty<std::vector<std::vector<std::vector<Real>>>> * const _dmfrac_dvar;

  /// Fluid component index for this kernel
  const unsigned int _fluid_component;

  using PorousFlowFullySaturatedDarcyBaseTempl<is_ad>::_dictator;
  using PorousFlowFullySaturatedDarcyBaseTempl<is_ad>::_multiply_by_density;
  using PorousFlowFullySaturatedDarcyBaseTempl<is_ad>::_density;
  using PorousFlowFullySaturatedDarcyBaseTempl<is_ad>::_viscosity;
  using PorousFlowFullySaturatedDarcyBaseTempl<is_ad>::_ddensity_dvar;
  using PorousFlowFullySaturatedDarcyBaseTempl<is_ad>::_dviscosity_dvar;
  usingGenericKernelMembers;
};

typedef PorousFlowFullySaturatedDarcyFlowTempl<false> PorousFlowFullySaturatedDarcyFlow;
typedef PorousFlowFullySaturatedDarcyFlowTempl<true> ADPorousFlowFullySaturatedDarcyFlow;
