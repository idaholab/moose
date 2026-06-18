//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowDarcyBase.h"

/**
 * Convective flux of component k in a single-phase fluid.
 * A fully-upwinded version is implemented, where the mobility
 * of the upstream nodes is used.
 *
 * Templated on is_ad: the false instantiation uses hand-coded Jacobians;
 * the true instantiation propagates derivatives through AD material properties.
 */
template <bool is_ad>
class PorousFlowFullySaturatedAdvectiveFluxTempl : public PorousFlowDarcyBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowFullySaturatedAdvectiveFluxTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> mobility(unsigned nodenum, unsigned phase) const override;
  virtual Real dmobility(unsigned nodenum, unsigned phase, unsigned pvar) const override;

  /// Mass fraction of each component in each phase
  const GenericMaterialProperty<std::vector<std::vector<Real>>, is_ad> & _mass_fractions;

  /// Derivative of mass fraction wrt PorousFlow variables -- null for AD path
  const MaterialProperty<std::vector<std::vector<std::vector<Real>>>> * const _dmass_fractions_dvar;

  /// Index of the fluid component that this kernel acts on
  const unsigned int _fluid_component;

  /// Whether the flux is multiplied by density (mass flux) or not (volume flux)
  const bool _multiply_by_density;

  using PorousFlowDarcyBaseTempl<is_ad>::_dictator;
  using PorousFlowDarcyBaseTempl<is_ad>::_fluid_density_node;
  using PorousFlowDarcyBaseTempl<is_ad>::_fluid_viscosity;
  using PorousFlowDarcyBaseTempl<is_ad>::_dfluid_density_node_dvar;
  using PorousFlowDarcyBaseTempl<is_ad>::_dfluid_viscosity_dvar;
  usingGenericKernelMembers;
};

typedef PorousFlowFullySaturatedAdvectiveFluxTempl<false> PorousFlowFullySaturatedAdvectiveFlux;
typedef PorousFlowFullySaturatedAdvectiveFluxTempl<true> ADPorousFlowFullySaturatedAdvectiveFlux;
