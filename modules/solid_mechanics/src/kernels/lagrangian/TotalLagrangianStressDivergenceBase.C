//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TotalLagrangianStressDivergenceBase.h"

template <class G>
InputParameters
TotalLagrangianStressDivergenceBase<G>::baseParams()
{
  InputParameters params = LagrangianStressDivergenceBase::validParams();
  // This kernel requires use_displaced_mesh to be off
  params.suppressParameter<bool>("use_displaced_mesh");
  return params;
}

template <class G>
TotalLagrangianStressDivergenceBase<G>::TotalLagrangianStressDivergenceBase(
    const InputParameters & parameters)
  : LagrangianStressDivergenceBase(parameters),
    _pk1(getMaterialPropertyByName<RankTwoTensor>(_base_name + "pk1_stress")),
    _dpk1(getMaterialPropertyByName<RankFourTensor>(_base_name + "pk1_jacobian")),
    _dpk1_d_grad_u(getMaterialPropertyByName<RankFourTensor>(_base_name + "dpk1_d_grad_u")),
    _dpk1_bypass_fbar(
        getMaterialPropertyByName<RankFourTensor>(_base_name + "pk1_jacobian_bypass_fbar"))
{
}

template <class G>
RankTwoTensor
TotalLagrangianStressDivergenceBase<G>::gradTest(unsigned int component)
{
  // F-bar doesn't modify the test function
  return G::gradOp(component, _grad_test[_i][_qp], _test[_i][_qp], _q_point[_qp]);
}

template <class G>
RankTwoTensor
TotalLagrangianStressDivergenceBase<G>::gradTrial(unsigned int component)
{
  // After the F_ust-wrap architectural change, pk1_jacobian = dPK1/d(F_ust) already
  // contains the local F-bar contribution (via the σ-chain through
  // `_d_F_stab_d_F_ust`). The trial gradient is always unstabilized; the non-local F-bar
  // Jacobian contribution is added explicitly in `computeQpJacobianDisplacement`.
  return gradTrialUnstabilized(component);
}

template <class G>
RankTwoTensor
TotalLagrangianStressDivergenceBase<G>::gradTrialUnstabilized(unsigned int component)
{
  // Without F-bar stabilization, simply return the gradient of the trial functions
  return G::gradOp(component, _grad_phi[_j][_qp], _phi[_j][_qp], _q_point[_qp]);
}

template <class G>
RankTwoTensor
TotalLagrangianStressDivergenceBase<G>::gradTrialStabilized(unsigned int component)
{
  // The base unstabilized trial function gradient
  const auto Gb = G::gradOp(component, _grad_phi[_j][_qp], _phi[_j][_qp], _q_point[_qp]);
  // The average trial function gradient
  const auto Ga = _avg_grad_trial[component][_j];

  // The F-bar stabilization depends on kinematics
  if (_large_kinematics)
  {
    // Horrible thing, see the documentation for how we get here
    const Real dratio = std::pow(_F_avg[_qp].det() / _F_ust[_qp].det(), 1.0 / 3.0);
    const Real fact = (_F_avg[_qp].inverse().transpose().doubleContraction(Ga) -
                       _F_ust[_qp].inverse().transpose().doubleContraction(Gb)) /
                      3.0;
    return dratio * (Gb + fact * _F_ust[_qp]);
  }

  // The small kinematics modification is linear
  return Gb + (Ga.trace() - Gb.trace()) / 3.0 * RankTwoTensor::Identity();
}

template <class G>
void
TotalLagrangianStressDivergenceBase<G>::precalculateJacobianDisplacement(unsigned int component)
{
  // For total Lagrangian, the averaging is taken on the reference frame regardless of geometric
  // nonlinearity. Convenient!
  for (auto j : make_range(_phi.size()))
    _avg_grad_trial[component][j] = StabilizationUtils::elementAverage(
        [this, component, j](unsigned int qp)
        { return G::gradOp(component, _grad_phi[j][qp], _phi[j][qp], _q_point[qp]); },
        _JxW,
        _coord);
}

template <class G>
Real
TotalLagrangianStressDivergenceBase<G>::computeQpResidual()
{
  return gradTest(_alpha).doubleContraction(_pk1[_qp]);
}

template <class G>
Real
TotalLagrangianStressDivergenceBase<G>::computeQpJacobianDisplacement(unsigned int alpha,
                                                                      unsigned int beta)
{
  // Local Jacobian: J_{alpha beta} = gradTest_α : (dPK1/d(grad u) · grad_phi_β). With
  // the F_ust-wrap architectural change, pk1_jacobian = dPK1/d(F_ust) carries the
  // local F-bar contribution through the σ chain (cauchy_jacobian · d(dL)/d(F_stab) ·
  // d(F_stab)/d(F_ust)).
  Real J = gradTest(alpha).doubleContraction(_dpk1_d_grad_u[_qp] * gradTrial(beta));

  // Non-local F-bar Jacobian contribution. PK1 = det(F_ust) · σ · F_ust^{-T} no longer
  // contains the non-local F-bar effect (it used to enter through F_stab in the wrap);
  // `deltaPK1NonLocalFBar` re-introduces it through the σ-via-dL chain. Guarded on
  // `_stabilize_strain` because `_avg_grad_trial` is only populated when F-bar is on.
  if (_stabilize_strain)
  {
    const RankTwoTensor delta_F_avg = _d_F_d_grad_u[_qp] * _avg_grad_trial[beta][_j];
    J += gradTest(alpha).doubleContraction(deltaPK1NonLocalFBar(delta_F_avg));
  }

  return J;
}

template <class G>
Real
TotalLagrangianStressDivergenceBase<G>::computeQpJacobianTemperature(unsigned int cvar)
{
  // Multiple eigenstrains may depend on the same coupled var
  RankTwoTensor total_deigen;
  for (const auto deigen_darg : _deigenstrain_dargs[cvar])
    total_deigen += (*deigen_darg)[_qp];

  // No eigenstrain → no temperature coupling. Short-circuit before dereferencing the
  // d_sigma/d_eigenstrain property (only fetched when eigenstrains are coupled; see
  // LagrangianStressDivergenceBase ctor).
  if (total_deigen.L2norm() == 0.0)
    return 0.0;

  // Direct chain through the constitutive update. The stress material publishes
  //   d_sigma/d_eigenstrain   (= -Jinv * small_jacobian for the objective-rate path,
  //                            with the sign convention that an eigenstrain increase
  //                            reduces the mechanical strain).
  // We then wrap to PK1 the same way the residual does:
  //   dP/dT = det(F) * dsigma/dT * F^{-T}   (large kinematics)
  //   dP/dT = dsigma/dT                     (small kinematics, P == sigma by convention)
  const RankTwoTensor dsigma_dT = (*_dcauchy_stress_d_eigenstrain)[_qp] * total_deigen;
  RankTwoTensor dP_dT;
  if (_large_kinematics)
    dP_dT = _F[_qp].det() * dsigma_dT * _F_inv[_qp].transpose();
  else
    dP_dT = dsigma_dT;
  return dP_dT.doubleContraction(gradTest(_alpha)) * _temperature->phi()[_j][_qp];
}

template <class G>
Real
TotalLagrangianStressDivergenceBase<G>::computeQpJacobianOutOfPlaneStrain()
{
  // d(R_disp_α)/d(strain_zz_j) at qp = gradTest_α : d(PK1)/d(strain_zz_j).
  // strain_zz feeds `_F[(2,2)]` AFTER F-bar runs in `ComputeLagrangianWPSStrain`, so
  // strain_zz perturbations bypass F-bar's chain. Use `_dpk1_bypass_fbar` (= the
  // pk1_jacobian variant computed with the F-bar `_d_F_stab_d_F_ust` factor REPLACED
  // by identity in the σ chain) for a consistent Jacobian.
  return _dpk1_bypass_fbar[_qp].contractionKl(2, 2, gradTest(_alpha)) *
         _out_of_plane_strain->phi()[_j][_qp];
}

template class TotalLagrangianStressDivergenceBase<GradientOperatorCartesian>;
template class TotalLagrangianStressDivergenceBase<GradientOperatorAxisymmetricCylindrical>;
template class TotalLagrangianStressDivergenceBase<GradientOperatorCentrosymmetricSpherical>;
