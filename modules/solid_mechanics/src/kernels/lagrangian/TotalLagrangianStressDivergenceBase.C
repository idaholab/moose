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
    _dpk1_d_grad_u(getMaterialPropertyByName<RankFourTensor>(_base_name + "dpk1_d_grad_u"))
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
  return _stabilize_strain ? gradTrialStabilized(component) : gradTrialUnstabilized(component);
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
  // J_{alpha beta} = phi^alpha_{i, J} T_{iJkL} G^beta_{kL}
  // _dpk1_d_grad_u = d(PK1)/d(grad u_{n+1}) already incorporates the generalized-alpha
  // kinematic policy via _d_F_d_grad_u (see ComputeLagrangianStressBase::computeQpProperties).
  return gradTest(alpha).doubleContraction(_dpk1_d_grad_u[_qp] * gradTrial(beta));
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
  return _dpk1[_qp].contractionKl(2, 2, gradTest(_alpha)) * _out_of_plane_strain->phi()[_j][_qp];
}

template class TotalLagrangianStressDivergenceBase<GradientOperatorCartesian>;
template class TotalLagrangianStressDivergenceBase<GradientOperatorAxisymmetricCylindrical>;
template class TotalLagrangianStressDivergenceBase<GradientOperatorCentrosymmetricSpherical>;
