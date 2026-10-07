//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowMaterialVectorBase.h"

/**
 * Base class Material designed to provide the porosity.
 */
template <bool is_ad>
class PorousFlowPorosityBaseTempl : public PorousFlowMaterialVectorBase
{
public:
  static InputParameters validParams();

  PorousFlowPorosityBaseTempl(const InputParameters & parameters);

protected:
  /**
   * If the user has provided porosity_min and _porosity[_qp] < porosity_min, set
   * _porosity[_qp] = porosity_min and multiply the derivatives of porosity by _zero_modifier (the
   * derivative material properties for non-AD objects, and the AD derivatives for AD objects).
   * Otherwise do nothing.  Derived classes call this at the end of computeQpProperties(), after
   * the unfloored porosity and its derivatives have been computed.
   */
  void applyPorosityMin();

  /// Computed porosity at the nodes or quadpoints
  GenericMaterialProperty<Real, is_ad> & _porosity;

  /// d(porosity)/d(PorousFlow variable)
  MaterialProperty<std::vector<Real>> * const _dporosity_dvar;

  /// d(porosity)/d(grad PorousFlow variable)
  MaterialProperty<std::vector<RealGradient>> * const _dporosity_dgradvar;

  /// Minimum allowed porosity.  Equals the lowest Real (so no floor is applied) if the user does not provide porosity_min
  const Real _porosity_min;

  /// When the porosity_min floor is active, the porosity derivatives are multiplied by this
  const Real _zero_modifier;
};

#define usingPorousFlowPorosityBaseMembers                                                         \
  using PorousFlowPorosityBaseTempl<is_ad>::_qp;                                                   \
  using PorousFlowPorosityBaseTempl<is_ad>::_num_var;                                              \
  using PorousFlowPorosityBaseTempl<is_ad>::_porosity;                                             \
  using PorousFlowPorosityBaseTempl<is_ad>::_dporosity_dvar;                                       \
  using PorousFlowPorosityBaseTempl<is_ad>::_dporosity_dgradvar;                                   \
  using PorousFlowPorosityBaseTempl<is_ad>::_porosity_min;                                         \
  using PorousFlowPorosityBaseTempl<is_ad>::_zero_modifier;                                        \
  using PorousFlowPorosityBaseTempl<is_ad>::applyPorosityMin;                                      \
  using Coupleable::coupledValue

typedef PorousFlowPorosityBaseTempl<false> PorousFlowPorosityBase;
typedef PorousFlowPorosityBaseTempl<true> ADPorousFlowPorosityBase;
