//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TotalLagrangianStressDivergenceBaseS.h"

template <class G>
InputParameters
TotalLagrangianStressDivergenceBaseS<G>::baseParams()
{
  InputParameters params = LagrangianStressDivergenceBaseS::validParams();
  // This kernel requires use_displaced_mesh to be off
  params.suppressParameter<bool>("use_displaced_mesh");
  return params;
}

template <class G>
TotalLagrangianStressDivergenceBaseS<G>::TotalLagrangianStressDivergenceBaseS(
    const InputParameters & parameters)
  : LagrangianStressDivergenceBaseS(parameters),
    _pk1(getMaterialPropertyByName<RankTwoTensor>(_base_name + "pk1_stress")),
    _dpk1(getMaterialPropertyByName<RankFourTensor>(_base_name + "pk1_jacobian"))
{
}

template <class G>
RankTwoTensor
TotalLagrangianStressDivergenceBaseS<G>::gradTest(unsigned int component)
{
  // F-bar doesn't modify the test function
  return G::gradOp(component, _grad_test[_i][_qp], _test[_i][_qp], _q_point[_qp]);
}

template <class G>
RankTwoTensor
TotalLagrangianStressDivergenceBaseS<G>::gradTrial(unsigned int component)
{
  return _stabilize_strain ? gradTrialStabilized(component) : gradTrialUnstabilized(component);
}

template <class G>
RankTwoTensor
TotalLagrangianStressDivergenceBaseS<G>::gradTrialUnstabilized(unsigned int component)
{
  // Without F-bar stabilization, simply return the gradient of the trial functions
  return G::gradOp(component, _grad_phi[_j][_qp], _phi[_j][_qp], _q_point[_qp]);
}

template <class G>
RankTwoTensor
TotalLagrangianStressDivergenceBaseS<G>::gradTrialStabilized(unsigned int component)
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
TotalLagrangianStressDivergenceBaseS<G>::precalculateJacobianDisplacement(unsigned int component)
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
TotalLagrangianStressDivergenceBaseS<G>::computeQpResidual()
{
  return gradTest(_alpha).doubleContraction(_pk1[_qp]);
}

template <class G>
Real
TotalLagrangianStressDivergenceBaseS<G>::computeQpJacobianDisplacement(unsigned int alpha,
                                                                       unsigned int beta)
{
  // J_{alpha beta} = phi^alpha_{i, J} T_{iJkL} G^beta_{kL}
  return gradTest(alpha).doubleContraction(_dpk1[_qp] * gradTrial(beta));
}

template <class G>
Real
TotalLagrangianStressDivergenceBaseS<G>::computeQpJacobianTemperature(unsigned int cvar)
{
  usingTensorIndices(i_, j_, k_, l_);
  // Multiple eigenstrains may depend on the same coupled var
  RankTwoTensor total_deigen;
  for (const auto deigen_darg : _deigenstrain_dargs[cvar])
    total_deigen += (*deigen_darg)[_qp];

  const auto A = _f_inv[_qp].inverse();
  const auto B = _F_inv[_qp].inverse();
  const auto U = 0.5 * (A.template times<i_, k_, l_, j_>(B) + A.template times<i_, l_, k_, j_>(B));

  return -(_dpk1[_qp] * U * total_deigen).doubleContraction(gradTest(_alpha)) *
         _temperature->phi()[_j][_qp];
}

template <class G>
Real
TotalLagrangianStressDivergenceBaseS<G>::computeQpJacobianOutOfPlaneStrain()
{
  return _dpk1[_qp].contractionKl(2, 2, gradTest(_alpha)) * _out_of_plane_strain->phi()[_j][_qp];
}

template class TotalLagrangianStressDivergenceBaseS<GradientOperatorCartesian>;
template class TotalLagrangianStressDivergenceBaseS<GradientOperatorAxisymmetricCylindrical>;
template class TotalLagrangianStressDivergenceBaseS<GradientOperatorCentrosymmetricSpherical>;
