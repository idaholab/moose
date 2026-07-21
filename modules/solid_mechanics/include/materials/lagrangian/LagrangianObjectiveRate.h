//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Inputs/Outputs hold RankTwoTensor / RankFourTensor by value, so the full definitions are needed.
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

class MooseEnum;

/// Objective-stress-rate updates as stateless free functions.
///
/// Each rate integrates the constitutive small-stress increment into the cumulative Cauchy stress
/// using its objective-rate advection and, when requested, returns the consistent tangent
/// d(sigma_cauchy)/d(dL) plus the eigenstrain sensitivity. The rates are pure functions of an
/// explicit `Inputs` bundle -- no coupling to the material that gathers them -- so they can be unit
/// tested in isolation and dispatched by a `switch` (composition, not a class hierarchy).
namespace LagrangianObjectiveRates
{
/// Per-qp quantities a rate reads. The host material gathers these once per qp from the strain
/// calculator's published properties and its own constitutive small stress. The Green-Naghdi-only
/// fields are default-constructed for the other rates -- only `greenNaghdi` reads them.
struct Inputs
{
  /// Constitutive small-stress increment (`_small_stress - _small_stress_old`).
  RankTwoTensor dS;
  /// Cumulative Cauchy stress at step n.
  RankTwoTensor cauchy_stress_old;
  /// Small-strain algorithmic tangent.
  RankFourTensor small_jacobian;
  /// Spatial velocity gradient increment dL and its vorticity (skew) part dW.
  RankTwoTensor dL, dW;
  /// d(dL)/dF and d(dW)/dF from the strain calculator.
  RankFourTensor d_dL_d_F, d_dW_d_F;
  /// Green-Naghdi only: polar-decomposition rotation R (n+1 and n), the inverse incremental
  /// deformation gradient, and F^{-1}.
  RankTwoTensor rotation, rotation_old, inv_df, inv_def_grad;
  /// Green-Naghdi only: dR/dF.
  RankFourTensor d_rotation_d_F;
};

/// Per-qp outputs a rate produces. `cauchy_jacobian` and `dcauchy_stress_d_eigenstrain` are filled
/// only when `need_jacobian` is true (they are read only on Jacobian sweeps).
struct Outputs
{
  RankTwoTensor cauchy_stress;
  RankFourTensor cauchy_jacobian;
  RankFourTensor dcauchy_stress_d_eigenstrain;
};

/// sigma_{n+1} = J(dL)^{-1} (sigma_n + Deltasigma). Truesdell rate.
Outputs truesdell(const Inputs & in, bool need_jacobian);
/// sigma_{n+1} = J(dW)^{-1} (sigma_n + Deltasigma). Jaumann rate.
Outputs jaumann(const Inputs & in, bool need_jacobian);
/// sigma_{n+1} = J(dO)^{-1} (sigma_n + Deltasigma), with dO built from the polar rotation increment.
Outputs greenNaghdi(const Inputs & in, bool need_jacobian);
/// sigma_{n+1} = r_hat (sigma_n + Deltasigma) r_hat^T, r_hat = exp(Deltaw). See
/// `rashid_project/plan_outline.pdf` Sec.3.2.
Outputs rashid(const Inputs & in, bool need_jacobian);

/// Dispatch to the rate selected by the `objective_rate` enum
/// (truesdell / jaumann / green_naghdi / rashid). Errors on an unknown value.
Outputs compute(const MooseEnum & rate, const Inputs & in, bool need_jacobian);
}
