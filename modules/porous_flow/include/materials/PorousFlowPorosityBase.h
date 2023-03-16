//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
  /// Computed porosity at the nodes or quadpoints
  GenericMaterialProperty<Real, is_ad> & _porosity;

  /// d(porosity)/d(PorousFlow variable)
  MaterialProperty<std::vector<Real>> * const _dporosity_dvar;

  /// d(porosity)/d(grad PorousFlow variable)
  MaterialProperty<std::vector<RealGradient>> * const _dporosity_dgradvar;
};

#define usingPorousFlowPorosityBaseMembers                                                         \
  using PorousFlowPorosityBaseTempl<is_ad>::_qp;                                                   \
  using PorousFlowPorosityBaseTempl<is_ad>::_num_var;                                              \
  using PorousFlowPorosityBaseTempl<is_ad>::_porosity;                                             \
  using PorousFlowPorosityBaseTempl<is_ad>::_dporosity_dvar;                                       \
  using PorousFlowPorosityBaseTempl<is_ad>::_dporosity_dgradvar;                                   \
  using Coupleable::coupledValue

typedef PorousFlowPorosityBaseTempl<false> PorousFlowPorosityBase;
typedef PorousFlowPorosityBaseTempl<true> ADPorousFlowPorosityBase;
