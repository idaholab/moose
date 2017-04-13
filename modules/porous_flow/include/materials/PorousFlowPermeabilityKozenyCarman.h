/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWPERMEABILITYKOZENYCARMAN_H
#define POROUSFLOWPERMEABILITYKOZENYCARMAN_H

#include "PorousFlowPermeabilityBase.h"

// Forward Declarations
class PorousFlowPermeabilityKozenyCarman;

template <>
InputParameters validParams<PorousFlowPermeabilityKozenyCarman>();

/**
 * Material designed to provide the permeability tensor which is calculated
 * from porosity using a form of the Kozeny-Carman equation (e.g. Oelkers
 * 1996: Reviews in Mineralogy v. 34, p. 131-192):
 * k = k_ijk * A * phi^n / (1 - phi)^m
 * where k_ijk is a tensor providing the anisotropy, phi is porosity,
 * n and m are positive scalar constants and A is given in one of the
 * following forms:
 * A = k0 * (1 - phi0)^m / phi0^n
 * where k0 and phi0 are a reference permeability and porosity,
 * or
 * A = f * d^2
 * where f is a scalar constant and d is grain diameter.
 */
class PorousFlowPermeabilityKozenyCarman : public PorousFlowPermeabilityBase
{
public:
  PorousFlowPermeabilityKozenyCarman(const InputParameters & parameters);

protected:
  void computeQpProperties() override;

  /// Reference scalar permeability in A = k0 * (1 - phi0)^m / phi0^n
  const Real _k0;

  /// Reference porosity in A = k0 * (1 - phi0)^m / phi0^n
  const Real _phi0;

  /// Multiplying factor in A = f * d^2
  const Real _f;

  /// Grain diameter in A = f * d^2
  const Real _d;

  /// Exponent in k = k_ijk * A * phi^n / (1 - phi)^m
  const Real _m;

  /// Exponent in k = k_ijk * A * phi^n / (1 - phi)^m
  const Real _n;

  /// Tensor multiplier k_ijk in k = k_ijk * A * phi^n / (1 - phi)^m
  const RealTensorValue _k_anisotropy;

  /// quadpoint porosity
  const MaterialProperty<Real> & _porosity_qp;

  /// d(quadpoint porosity)/d(PorousFlow variable)
  const MaterialProperty<std::vector<Real>> & _dporosity_qp_dvar;

  /// d(quadpoint porosity)/d(grad(PorousFlow variable))
  const MaterialProperty<std::vector<RealGradient>> & _dporosity_qp_dgradvar;

  /// Name of porosity-permeability relationship
  const enum PoropermFunction { kozeny_carman_fd2, kozeny_carman_phi0 } _poroperm_function;

  /// Multiplying factor in k = k_ijk * A * phi^n / (1 - phi)^m
  Real _A;
};

#endif // POROUSFLOWPERMEABILITYKOZENYCARMAN_H
