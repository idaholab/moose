//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "UpdatedLagrangianStressDivergence.h"

registerMooseObject("TensorMechanicsApp", UpdatedLagrangianStressDivergence);

template <class G>
UpdatedLagrangianStressDivergenceBase<G>::UpdatedLagrangianStressDivergenceBase(
    const InputParameters & parameters)
  : LagrangianStressDivergenceBase(parameters),
    _stress(getMaterialPropertyByName<RankTwoTensor>(_base_name + "cauchy_stress")),
    _material_jacobian(getMaterialPropertyByName<RankFourTensor>(_base_name + "cauchy_jacobian")),

    // Assembly quantities in the reference frame for stabilization
    _assembly_undisplaced(_fe_problem.assembly(_tid)),
    _grad_phi_undisplaced(_assembly_undisplaced.gradPhi()),
    _JxW_undisplaced(_assembly_undisplaced.JxW()),
    _coord_undisplaced(_assembly_undisplaced.coordTransformation()),
    _q_point_undisplaced(_assembly_undisplaced.qPoints())
{
  // This kernel requires used_displaced_mesh to be true if large kinematics
  // is on
  if (_large_kinematics && (!getParam<bool>("use_displaced_mesh")))
    mooseError("The UpdatedLagrangianStressDivergence kernels requires "
               "used_displaced_mesh = true for large_kinematics = true");

  // Similarly, if large kinematics is off so should use_displaced_mesh
  if (!_large_kinematics && (getParam<bool>("use_displaced_mesh")))
    mooseError("The UpdatedLagrangianStressDivergence kernels requires "
               "used_displaced_mesh = false for large_kinematics = false");

  // TODO: add weak plane stress support
  if (_out_of_plane_strain)
    mooseError("The UpdatedLagrangianStressDivergence kernels do not yet support the weak plane "
               "stress formulation. Please use the TotalLagrangianStressDivergecen kernels.");
}

template <class G>
RankTwoTensor
UpdatedLagrangianStressDivergenceBase<G>::gradTest(unsigned int component)
{
  // F-bar doesn't modify the test function
  return G::gradOp(component, _grad_test[_i][_qp], _test[_i][_qp], _q_point[_qp]);
}

template <class G>
RankTwoTensor
UpdatedLagrangianStressDivergenceBase<G>::gradTrial(unsigned int component)
{
  return _stabilize_strain ? gradTrialStabilized(component) : gradTrialUnstabilized(component);
}

template <class G>
RankTwoTensor
UpdatedLagrangianStressDivergenceBase<G>::gradTrialUnstabilized(unsigned int component)
{
  // Without F-bar stabilization, simply return the gradient of the trial functions
  return G::gradOp(component, _grad_phi[_j][_qp], _phi[_j][_qp], _q_point[_qp]);
}

template <class G>
RankTwoTensor
UpdatedLagrangianStressDivergenceBase<G>::gradTrialStabilized(unsigned int component)
{
  // The base unstabilized trial function gradient
  const auto Gb = G::gradOp(component, _grad_phi[_j][_qp], _phi[_j][_qp], _q_point[_qp]);
  // The average trial function gradient
  const auto Ga = _avg_grad_trial[component][_j];
  // For updated Lagrangian, the modification is always linear
  return Gb + (Ga.trace() - Gb.trace()) / 3.0 * RankTwoTensor::Identity();
}

template <class G>
void
UpdatedLagrangianStressDivergenceBase<G>::precalculateJacobianDisplacement(unsigned int component)
{
  // For updated Lagrangian, the averaging is taken on the reference frame. If large kinematics is
  // used, the averaged gradients should be pushed forward to the current frame.
  for (auto j : make_range(_phi.size()))
  {
    _avg_grad_trial[component][j] = StabilizationUtils::elementAverage(
        [this, component, j](unsigned int qp)
        {
          return G::gradOp(
              component, _grad_phi_undisplaced[j][qp], _phi[j][qp], _q_point_undisplaced[qp]);
        },
        _JxW_undisplaced,
        _coord_undisplaced);
    if (_large_kinematics)
      // Push forward to the current frame.
      // The average deformation gradient is the same at all qps.
      _avg_grad_trial[component][j] *= _F_avg[0].inverse();
  }
}

template <class G>
Real
UpdatedLagrangianStressDivergenceBase<G>::computeQpResidual()
{
  return gradTest(_alpha).doubleContraction(_stress[_qp]);
}

template <class G>
Real
UpdatedLagrangianStressDivergenceBase<G>::computeQpJacobianDisplacement(unsigned int alpha,
                                                                        unsigned int beta)
{
  const auto grad_test = gradTest(alpha);
  const auto grad_trial = gradTrial(beta);

  //           J^{alpha beta} = J^{alpha beta}_material + J^{alpha beta}_geometric
  //  J^{alpha beta}_material = phi^alpha_{i, j} T_{ijkl} f^{-1}_{km} g^beta_{ml}
  // J^{alpha beta}_geometric = sigma_{ij} (phi^alpha_{k, k} psi^beta_{i, j} -
  //                                        phi^alpha_{k, j} psi^beta_{i, k})

  // The material jacobian
  Real J = grad_test.doubleContraction(_material_jacobian[_qp] * (_f_inv[_qp] * grad_trial));

  // The geometric jacobian
  if (_large_kinematics)
  {
    // No stablization in the geometric jacobian
    const auto grad_trial_ust = gradTrialUnstabilized(beta);
    J += _stress[_qp].doubleContraction(grad_test) * grad_trial_ust.trace() -
         _stress[_qp].doubleContraction(grad_test * grad_trial_ust);
  }

  return J;
}

template <class G>
Real
UpdatedLagrangianStressDivergenceBase<G>::computeQpJacobianTemperature(unsigned int cvar)
{
  // Multiple eigenstrains may depend on the same coupled var
  RankTwoTensor total_deigen;
  for (const auto deigen_darg : _deigenstrain_dargs[cvar])
    total_deigen += (*deigen_darg)[_qp];

  RankFourTensor C = _material_jacobian[_qp];
  RankFourTensor Csym = 0.5 * (C + C.transposeMajor().transposeIj().transposeMajor());

  return -(Csym * total_deigen).doubleContraction(gradTest(_alpha)) * _temperature->phi()[_j][_qp];
}

template class UpdatedLagrangianStressDivergenceBase<GradientOperatorCartesian>;
