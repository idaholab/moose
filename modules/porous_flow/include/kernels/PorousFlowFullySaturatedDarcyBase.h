//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericKernel.h"
#include "PorousFlowDictator.h"

/**
 * Darcy advective flux for a fully-saturated, single-phase, single-component
 * fluid.  No upwinding or relative-permeability effects are included.
 *
 * Templated on is_ad: the false instantiation uses hand-coded Jacobians;
 * the true instantiation propagates derivatives through AD material properties.
 */
template <bool is_ad>
class PorousFlowFullySaturatedDarcyBaseTempl : public GenericKernel<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowFullySaturatedDarcyBaseTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// Fluid mobility = (density *) 1/viscosity.  Virtual so DarcyFlow can multiply by mass frac.
  virtual GenericReal<is_ad> mobility() const;

  /// d(mobility)/d(PorousFlow variable pvar) -- only used on the non-AD path
  virtual Real dmobility(unsigned int pvar) const;

  /// If true the mobility includes fluid density (mass flux); otherwise volume flux
  const bool _multiply_by_density;

  /// Permeability tensor
  const GenericMaterialProperty<RealTensorValue, is_ad> & _permeability;

  /// d(permeability)/d(PorousFlow variable) -- null for AD path
  const MaterialProperty<std::vector<RealTensorValue>> * const _dpermeability_dvar;

  /// d(permeability)/d(grad PorousFlow variable) -- null for AD path
  const MaterialProperty<std::vector<std::vector<RealTensorValue>>> * const _dpermeability_dgradvar;

  /// Fluid density for each phase
  const GenericMaterialProperty<std::vector<Real>, is_ad> & _density;

  /// d(density)/d(PorousFlow variable) -- null for AD path
  const MaterialProperty<std::vector<std::vector<Real>>> * const _ddensity_dvar;

  /// Fluid viscosity for each phase
  const GenericMaterialProperty<std::vector<Real>, is_ad> & _viscosity;

  /// d(viscosity)/d(PorousFlow variable) -- null for AD path
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dviscosity_dvar;

  /// Quadpoint pore pressure for each phase
  const GenericMaterialProperty<std::vector<Real>, is_ad> & _pp;

  /// Gradient of pore pressure for each phase
  const GenericMaterialProperty<std::vector<RealGradient>, is_ad> & _grad_p;

  /// d(grad p)/d(grad PorousFlow variable) -- null for AD path
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dgrad_p_dgrad_var;

  /// d(grad p)/d(PorousFlow variable) -- null for AD path
  const MaterialProperty<std::vector<std::vector<RealGradient>>> * const _dgrad_p_dvar;

  /// PorousFlowDictator UserObject
  const PorousFlowDictator & _dictator;

  /// Gravitational acceleration vector (downward)
  const RealVectorValue _gravity;

  /// Whether permeability has non-zero derivatives w.r.t. primary variables
  const bool _perm_derivs;

  usingGenericKernelMembers;
  using GenericKernel<is_ad>::_grad_phi;
};

typedef PorousFlowFullySaturatedDarcyBaseTempl<false> PorousFlowFullySaturatedDarcyBase;
typedef PorousFlowFullySaturatedDarcyBaseTempl<true> ADPorousFlowFullySaturatedDarcyBase;
