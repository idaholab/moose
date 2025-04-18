//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowPermeabilityBase.h"

/**
 * Base class for material designed to provide the permeability tensor which is calculated
 * from porosity using a form of the Kozeny-Carman equation (e.g. Oelkers
 * 1996: Reviews in Mineralogy v. 34, p. 131-192):
 * k = k_ijk * A * phi^n / (1 - phi)^m
 * where k_ijk is a tensor providing the anisotropy, phi is porosity,
 * n and m are positive scalar constants.
 * Methods for computing A are in the derived classes
 */
template <bool is_ad>
class PorousFlowPermeabilityKozenyCarmanBaseTempl : public PorousFlowPermeabilityBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowPermeabilityKozenyCarmanBaseTempl(const InputParameters & parameters);

protected:
  void computeQpProperties() override;

  /// function for returning A factor in Kozeny Carman equation
  virtual Real computeA() const = 0;

  /// Exponent in k = k_ijk * A * phi^n / (1 - phi)^m
  const Real _m;

  /// Exponent in k = k_ijk * A * phi^n / (1 - phi)^m
  const Real _n;

  /// Tensor multiplier k_ijk in k = k_ijk * A * phi^n / (1 - phi)^m
  const RealTensorValue _k_anisotropy;

  /// Quadpoint porosity
  const GenericMaterialProperty<Real, is_ad> & _porosity_qp;

  /// d(quadpoint porosity)/d(PorousFlow variable)
  const MaterialProperty<std::vector<Real>> * const _dporosity_qp_dvar;

  /// d(quadpoint porosity)/d(grad(PorousFlow variable))
  const MaterialProperty<std::vector<RealGradient>> * const _dporosity_qp_dgradvar;

  usingPorousFlowPermeabilityBaseMembers;
};

typedef PorousFlowPermeabilityKozenyCarmanBaseTempl<false> PorousFlowPermeabilityKozenyCarmanBase;
typedef PorousFlowPermeabilityKozenyCarmanBaseTempl<true> ADPorousFlowPermeabilityKozenyCarmanBase;
