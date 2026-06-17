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
 * Advection of heat via flux via Darcy flow of a single phase
 * fully-saturated fluid.  No upwinding is used.
 *
 * Templated on is_ad: the false instantiation uses hand-coded Jacobians;
 * the true instantiation propagates derivatives through AD material properties.
 */
template <bool is_ad>
class PorousFlowFullySaturatedHeatAdvectionTempl
  : public PorousFlowFullySaturatedDarcyBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowFullySaturatedHeatAdvectionTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> mobility() const override;
  virtual Real dmobility(unsigned pvar) const override;

  /// Enthalpy of each phase
  const GenericMaterialProperty<std::vector<Real>, is_ad> & _enthalpy;

  /// Derivative of enthalpy wrt PorousFlow variables -- null for AD path
  const MaterialProperty<std::vector<std::vector<Real>>> * const _denthalpy_dvar;

  usingGenericKernelMembers;
};

typedef PorousFlowFullySaturatedHeatAdvectionTempl<false> PorousFlowFullySaturatedHeatAdvection;
typedef PorousFlowFullySaturatedHeatAdvectionTempl<true> ADPorousFlowFullySaturatedHeatAdvection;
