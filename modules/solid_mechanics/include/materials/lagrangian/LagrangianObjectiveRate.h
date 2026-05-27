//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "RankTwoTensorForward.h"
#include "RankFourTensorForward.h"
#include <memory>
#include <tuple>

class ComputeLagrangianObjectiveStress;
class MooseEnum;

/// Abstract base class for the objective-stress-rate strategy.
///
/// The host material `ComputeLagrangianObjectiveStress` owns a unique_ptr to
/// one concrete subclass, selected by the `objective_rate` enum. Each subclass
/// reads kinematic / constitutive material-property handles off the host and
/// writes `_cauchy_stress`, `_cauchy_jacobian`, and
/// `_dcauchy_stress_d_eigenstrain` at the host's current `_qp`.
class LagrangianObjectiveRate
{
public:
  virtual ~LagrangianObjectiveRate() = default;

  /// Perform the objective stress update at the host's current quadrature point.
  /// `dS` is the constitutive small-stress increment (`_small_stress -
  /// _small_stress_old`). When `need_jacobian` is false the rate computes
  /// `_cauchy_stress` only and skips the RankFourTensor algebra feeding
  /// `_cauchy_jacobian` / `_dcauchy_stress_d_eigenstrain` -- both are read only on
  /// Jacobian sweeps.
  virtual void update(ComputeLagrangianObjectiveStress & host,
                      const RankTwoTensor & dS,
                      bool need_jacobian) const = 0;
};

/// Intermediate base for rates that fit the linear template
/// `sigma_{n+1} = J(dQ)^{-1} (sigma_n + Deltasigma)`. Provides the shared advection helpers.
class LagrangianLinearObjectiveRate : public LagrangianObjectiveRate
{
protected:
  /// Apply the linear advection to a stress, returning `(advected_stress, Jinv)`.
  static std::tuple<RankTwoTensor, RankFourTensor> advectStress(const RankTwoTensor & S0,
                                                                const RankTwoTensor & dQ);

  /// Build the J tensor that defines the linear advection.
  static RankFourTensor updateTensor(const RankTwoTensor & dQ);

  /// Derivative of the linear advection action with respect to the kinematic tensor.
  static RankFourTensor stressAdvectionDerivative(const RankTwoTensor & S);

  /// Consistent tangent for the linear template.
  static RankFourTensor cauchyJacobian(const RankFourTensor & Jinv,
                                       const RankFourTensor & small_jacobian,
                                       const RankFourTensor & U);
};

class LagrangianTruesdellRate : public LagrangianLinearObjectiveRate
{
public:
  void update(ComputeLagrangianObjectiveStress & host,
              const RankTwoTensor & dS,
              bool need_jacobian) const override;
};

class LagrangianJaumannRate : public LagrangianLinearObjectiveRate
{
public:
  void update(ComputeLagrangianObjectiveStress & host,
              const RankTwoTensor & dS,
              bool need_jacobian) const override;
};

class LagrangianGreenNaghdiRate : public LagrangianLinearObjectiveRate
{
public:
  void update(ComputeLagrangianObjectiveStress & host,
              const RankTwoTensor & dS,
              bool need_jacobian) const override;
};

/// Rashid rate: sigma_{n+1} = r_hat (sigma_n + Deltasigma) r_hat^T  with r_hat = exp(Deltaw).
/// See `rashid_project/plan_outline.pdf` Sec.3.2.
class LagrangianRashidRate : public LagrangianObjectiveRate
{
public:
  void update(ComputeLagrangianObjectiveStress & host,
              const RankTwoTensor & dS,
              bool need_jacobian) const override;

private:
  /// Rodrigues exponential of a skew 3x3 tensor W. When `dR_dW` is non-null it is filled
  /// with the unconstrained chain-rule derivative; otherwise the R4 algebra is skipped.
  /// The upstream `_d_vorticity_increment_d_F` projects out non-skew perturbations.
  static RankTwoTensor rotationFromVorticity(const RankTwoTensor & W, RankFourTensor * dR_dW);
};

/// Build the concrete rate strategy that matches the user's enum choice.
std::unique_ptr<LagrangianObjectiveRate> createObjectiveRate(const MooseEnum & rate_enum);
