//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

/// Publishes an isotropic Saint-Venant-Kirchhoff PK2 stress and dPK2/dF analytically. Used to
/// exercise the `ComputeLagrangianStressCustomPK2` wrap path (which is otherwise only fed by
/// NEML2) with a Jacobian-tester-friendly built-in material.
///
///   E_kl     = 0.5 (F_mk F_ml - delta_kl)
///   PK2_ij   = 2 mu E_ij + lambda tr(E) delta_ij
///   dPK2_ij  /dF_pq = mu (delta_iq F_pj + F_pi delta_jq) + lambda delta_ij F_pq.
class StVenantKirchhoffPK2Test : public Material
{
public:
  static InputParameters validParams();
  StVenantKirchhoffPK2Test(const InputParameters & parameters);

protected:
  void computeQpProperties() override;

  const Real _lambda;
  const Real _mu;

  /// Strain-calc-published F (= F-bar-stabilized when F-bar is on; equal to F_ust otherwise).
  /// PK2 is computed from this F so the constitutive law sees the stabilized strain.
  const MaterialProperty<RankTwoTensor> & _F;

  MaterialProperty<RankTwoTensor> & _pk2;
  MaterialProperty<RankFourTensor> & _dpk2_dF;
};
