//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "UpdatedLagrangianStressDivergence.h"

registerMooseObject("SolidMechanicsApp", UpdatedLagrangianStressDivergence);

template <class G>
UpdatedLagrangianStressDivergenceBase<G>::UpdatedLagrangianStressDivergenceBase(
    const InputParameters & parameters)
  : LagrangianStressDivergenceBase(parameters),
    _stress(getMaterialPropertyByName<RankTwoTensor>(_base_name + "cauchy_stress")),
    _material_jacobian(getMaterialPropertyByName<RankFourTensor>(_base_name + "cauchy_jacobian")),

    // Assembly quantities in the reference frame for stabilization
    _assembly_undisplaced(_fe_problem.assembly(_tid, this->_sys.number())),
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
  // F-bar stabilization is handled explicitly in computeQpJacobianDisplacement via the stored
  // partial derivatives _d_F_stab_d_F_ust and _d_F_stab_d_F_avg, so gradTrial always returns
  // the unstabilized spatial gradient.
  return gradTrialUnstabilized(component);
}

template <class G>
RankTwoTensor
UpdatedLagrangianStressDivergenceBase<G>::gradTrialUnstabilized(unsigned int component)
{
  // Without F-bar stabilization, simply return the gradient of the trial functions
  return G::gradOp(component, _grad_phi[_j][_qp], _phi[_j][_qp], _q_point[_qp]);
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
  const auto grad_trial = gradTrialUnstabilized(beta);

  //           J^{alpha beta} = J^{alpha beta}_material + J^{alpha beta}_geometric
  //  J^{alpha beta}_material = phi^alpha : T : d(dL)/d(grad u) * grad_trial
  // J^{alpha beta}_geometric = sigma_{ij} (phi^alpha_{k, k} psi^beta_{i, j} -
  //                                        phi^alpha_{k, j} psi^beta_{i, k})

  // Local contribution to delta(F_ust): pull the spatial trial gradient back through the
  // literal n+1 deformation gradient (use_displaced_mesh = true uses F_actual, regardless
  // of alpha or F-bar), then chain to F_ust via _d_F_d_grad_u (= alpha * I^(4)).
  const RankTwoTensor delta_grad_u_local =
      _large_kinematics ? grad_trial * _F_actual[_qp] : grad_trial;
  const RankTwoTensor delta_F_ust_local = _d_F_d_grad_u[_qp] * delta_grad_u_local;

  // Non-local F-bar contribution to delta(F_avg): _avg_grad_trial is stored as
  // avg(gradTrial_reference) * F_avg^{-1} (see precalculateJacobianDisplacement). Multiply
  // by F_avg to recover the reference-frame averaged gradient, then chain to F_ust the
  // same way.
  RankTwoTensor delta_F_avg;
  if (_stabilize_strain)
  {
    const RankTwoTensor delta_grad_u_avg =
        _large_kinematics ? _avg_grad_trial[beta][_j] * _F_avg[_qp] : _avg_grad_trial[beta][_j];
    delta_F_avg = _d_F_d_grad_u[_qp] * delta_grad_u_avg;
  }

  // Stabilized delta(F_stab) through the F-bar tangent. With F-bar off, the partials are
  // _d_F_stab_d_F_ust = I^(4) and _d_F_stab_d_F_avg = 0, so this reduces to delta_F_ust_local.
  const RankTwoTensor delta_F_stab =
      _d_F_stab_d_F_ust[_qp] * delta_F_ust_local + _d_F_stab_d_F_avg[_qp] * delta_F_avg;

  const RankTwoTensor delta_dL = _d_spatial_velocity_increment_d_F[_qp] * delta_F_stab;

  // The material jacobian
  Real J = grad_test.doubleContraction(_material_jacobian[_qp] * delta_dL);

  // The geometric jacobian (no F-bar stabilization in this term)
  if (_large_kinematics)
  {
    J += _stress[_qp].doubleContraction(grad_test) * grad_trial.trace() -
         _stress[_qp].doubleContraction(grad_test * grad_trial);
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
