//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowPermeabilityKozenyCarmanBase.h"

/**
 * Material designed to provide the permeability tensor which is calculated
 * from porosity using a form of the Kozeny-Carman equation (e.g. Oelkers
 * 1996: Reviews in Mineralogy v. 34, p. 131-192):
 * k = k_ijk * A * phi^n / (1 - phi)^m
 * where k_ijk is a tensor providing the anisotropy, phi is porosity,
 * n and m are positive scalar constants.
 * A is constant and is given in one of the following forms:
 * A = k0 * (1 - phi0)^m / phi0^n
 * where k0 and phi0 are a reference permeability and porosity,
 * or
 * A = f * d^2
 * where f is a scalar constant and d is grain diameter.
 */
template <bool is_ad>
class PorousFlowPermeabilityKozenyCarmanTempl
  : public PorousFlowPermeabilityKozenyCarmanBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowPermeabilityKozenyCarmanTempl(const InputParameters & parameters);

protected:
  /// retrieve constant value for A computed in constructor
  Real computeA() const override;

  /// Reference scalar permeability in A = k0 * (1 - phi0)^m / phi0^n
  const Real _k0;

  /// Reference porosity in A = k0 * (1 - phi0)^m / phi0^n
  const Real _phi0;

  /// Multiplying factor in A = f * d^2
  const Real _f;

  /// Grain diameter in A = f * d^2
  const Real _d;

  /// Name of porosity-permeability relationship
  const enum class PoropermFunction {
    kozeny_carman_fd2,
    kozeny_carman_phi0,
    kozeny_carman_A
  } _poroperm_function;

  /// Multiplying factor in k = k_ijk * A * phi^n / (1 - phi)^m
  Real _A;

  usingPorousFlowPermeabilityBaseMembers;
};

typedef PorousFlowPermeabilityKozenyCarmanTempl<false> PorousFlowPermeabilityKozenyCarman;
typedef PorousFlowPermeabilityKozenyCarmanTempl<true> ADPorousFlowPermeabilityKozenyCarman;
