//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowPorosity.h"

/**
 * This Matrial evolves porosity so that the PorousFlow equations match
 * the standard equations of poroelasticity theory with a constant BiotModulus.
 * Even though a constant BiotModulus is not strictly correct, many
 * analytical solutions of poroelasticity theory assume this, so
 * PorousFlowPorosityHMBiotModulus is useful for comparing with those solutions.
 * Otherwise it shouldn't generally be used because PorousFlowPorosity
 * is physically more correct.
 */
class PorousFlowPorosityHMBiotModulus : public PorousFlowPorosity
{
public:
  static InputParameters validParams();

  PorousFlowPorosityHMBiotModulus(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Old value of porosity
  const MaterialProperty<Real> & _porosity_old;

  /// Constant biot modulus
  const Real _biot_modulus;

  /// Constant fluid bulk modulus
  const Real _fluid_bulk_modulus;

  /// Old value of effective fluid pressure
  const MaterialProperty<Real> & _pf_old;

  /// Old value of total volumetric strain
  const MaterialProperty<Real> & _vol_strain_qp_old;

  /// Volumetric strain rate
  const MaterialProperty<Real> & _vol_strain_rate_qp;

  /// d(volumetric strain rate)/d(PorousFlow variable)
  const MaterialProperty<std::vector<RealGradient>> & _dvol_strain_rate_qp_dvar;
};
