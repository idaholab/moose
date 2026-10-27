//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowLumpedKernelBase.h"
#include "PorousFlowDictator.h"

/**
 * Kernel = sum (stoichiometry * density * porosity_old * saturation * reaction_rate)
 * where the sum is over secondary chemical species in
 * a precipitation-dissolution reaction system.
 *
 * Templated on is_ad: the false instantiation uses hand-coded Jacobians;
 * the true instantiation propagates derivatives through AD material properties.
 */
template <bool is_ad>
class PorousFlowPreDisTempl : public PorousFlowLumpedKernelBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowPreDisTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// Derivative of residual wrt PorousFlow variable pvar (non-AD path only)
  Real computeQpJac(unsigned int pvar);

  /// Density of the mineral species
  const std::vector<Real> _mineral_density;

  /// PorousFlowDictator UserObject
  const PorousFlowDictator & _dictator;

  /// Aqueous phase number
  const unsigned int _aq_ph;

  /// Old value of porosity (always non-AD: old values carry no current-timestep derivatives)
  const MaterialProperty<Real> & _porosity_old;

  /// Saturation (AD or non-AD)
  const GenericMaterialProperty<std::vector<Real>, is_ad> & _saturation;

  /// d(saturation)/d(PorousFlow var) -- null for AD path
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dsaturation_dvar;

  /// Reaction rate of the yielding the secondary species (AD or non-AD)
  const GenericMaterialProperty<std::vector<Real>, is_ad> & _reaction_rate;

  /// d(reaction rate)/d(PorousFlow variable) -- null for AD path
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dreaction_rate_dvar;

  /// Stoichiometric coefficients
  const std::vector<Real> _stoichiometry;

  usingGenericKernelMembers;
};

typedef PorousFlowPreDisTempl<false> PorousFlowPreDis;
typedef PorousFlowPreDisTempl<true> ADPorousFlowPreDis;
