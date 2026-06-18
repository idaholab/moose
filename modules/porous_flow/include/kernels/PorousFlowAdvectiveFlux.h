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
 * Convective flux of component k in fluid phase alpha.
 * A fully-updwinded version is implemented, where the mobility
 * of the upstream nodes is used.
 *
 * Templated on is_ad: the false instantiation uses hand-coded Jacobians;
 * the true instantiation propagates derivatives through AD material properties.
 */
template <bool is_ad>
class PorousFlowAdvectiveFluxTempl : public PorousFlowDarcyBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowAdvectiveFluxTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> mobility(unsigned nodenum, unsigned phase) const override;
  virtual Real dmobility(unsigned nodenum, unsigned phase, unsigned pvar) const override;

  /// Mass fraction of each component in each phase
  const GenericMaterialProperty<std::vector<std::vector<Real>>, is_ad> & _mass_fractions;

  /// Derivative of the mass fraction of each component in each phase wrt PorousFlow variables -- null for AD path
  const MaterialProperty<std::vector<std::vector<std::vector<Real>>>> * const _dmass_fractions_dvar;

  /// Relative permeability of each phase
  const GenericMaterialProperty<std::vector<Real>, is_ad> & _relative_permeability;

  /// Derivative of relative permeability of each phase wrt PorousFlow variables -- null for AD path
  const MaterialProperty<std::vector<std::vector<Real>>> * const _drelative_permeability_dvar;

  /// Index of the fluid component that this kernel acts on
  const unsigned int _fluid_component;

  using PorousFlowDarcyBaseTempl<is_ad>::_dictator;
  using PorousFlowDarcyBaseTempl<is_ad>::_fluid_density_node;
  using PorousFlowDarcyBaseTempl<is_ad>::_fluid_viscosity;
  using PorousFlowDarcyBaseTempl<is_ad>::_dfluid_density_node_dvar;
  using PorousFlowDarcyBaseTempl<is_ad>::_dfluid_viscosity_dvar;
  usingGenericKernelMembers;
};

typedef PorousFlowAdvectiveFluxTempl<false> PorousFlowAdvectiveFlux;
typedef PorousFlowAdvectiveFluxTempl<true> ADPorousFlowAdvectiveFlux;
