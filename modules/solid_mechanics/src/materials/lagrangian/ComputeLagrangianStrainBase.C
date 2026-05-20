//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLagrangianStrain.h"

template <class G>
InputParameters
ComputeLagrangianStrainBase<G>::baseParams()
{
  InputParameters params = Material::validParams();

  params.addRequiredCoupledVar("displacements", "Displacement variables");
  params.addParam<bool>(
      "large_kinematics", false, "Use large displacement kinematics in the kernel.");
  params.addParam<bool>("stabilize_strain", false, "Average the volumetric strains");
  params.addRangeCheckedParam<Real>(
      "alpha",
      1.0,
      "alpha >= 0.5 & alpha <= 1.0",
      "Generalized midpoint weight for the deformation gradient. 1.0 = backward Euler (default), "
      "0.5 = midpoint rule (matches Abaqus/Implicit).");
  params.addParam<std::vector<MaterialPropertyName>>(
      "eigenstrain_names", {}, "List of eigenstrains to account for");
  params.addParam<std::vector<MaterialPropertyName>>(
      "homogenization_gradient_names",
      {},
      "List of homogenization gradients to add to the displacement gradient");

  params.addParam<std::string>("base_name", "Material property base name");

  // We rely on this *not* having use_displaced mesh on
  params.suppressParameter<bool>("use_displaced_mesh");

  return params;
}

template <class G>
ComputeLagrangianStrainBase<G>::ComputeLagrangianStrainBase(const InputParameters & parameters)
  : Material(parameters),
    _ndisp(coupledComponents("displacements")),
    _disp(coupledValues("displacements")),
    _grad_disp(coupledGradients("displacements")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _large_kinematics(getParam<bool>("large_kinematics")),
    _stabilize_strain(getParam<bool>("stabilize_strain")),
    _alpha(getParam<Real>("alpha")),
    _eigenstrain_names(getParam<std::vector<MaterialPropertyName>>("eigenstrain_names")),
    _eigenstrains(_eigenstrain_names.size()),
    _eigenstrains_old(_eigenstrain_names.size()),
    _total_strain(declareProperty<RankTwoTensor>(_base_name + "total_strain")),
    _total_strain_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "total_strain")),
    _mechanical_strain(declareProperty<RankTwoTensor>(_base_name + "mechanical_strain")),
    _mechanical_strain_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "mechanical_strain")),
    _strain_increment(declareProperty<RankTwoTensor>(_base_name + "strain_increment")),
    _spatial_velocity_increment(
        declareProperty<RankTwoTensor>(_base_name + "spatial_velocity_increment")),
    _vorticity_increment(declareProperty<RankTwoTensor>(_base_name + "vorticity_increment")),
    _F_ust(declareProperty<RankTwoTensor>(_base_name + "unstabilized_deformation_gradient")),
    _F_actual(declareProperty<RankTwoTensor>(_base_name + "actual_deformation_gradient")),
    _F_avg(declareProperty<RankTwoTensor>(_base_name + "average_deformation_gradient")),
    _F(declareProperty<RankTwoTensor>(_base_name + "deformation_gradient")),
    _F_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "deformation_gradient")),
    _F_inv(declareProperty<RankTwoTensor>(_base_name + "inverse_deformation_gradient")),
    _f_inv(declareProperty<RankTwoTensor>(_base_name + "inverse_incremental_deformation_gradient")),
    _d_spatial_velocity_increment_d_F(declareProperty<RankFourTensor>(
        _base_name + "d_spatial_velocity_increment_d_deformation_gradient")),
    _d_F_d_grad_u(declareProperty<RankFourTensor>(
        _base_name + "d_deformation_gradient_d_grad_displacement")),
    _d_F_stab_d_F_ust(declareProperty<RankFourTensor>(
        _base_name + "d_F_stab_d_F_unstabilized")),
    _d_F_stab_d_F_avg(declareProperty<RankFourTensor>(
        _base_name + "d_F_stab_d_F_average")),
    _homogenization_gradient_names(
        getParam<std::vector<MaterialPropertyName>>("homogenization_gradient_names")),
    _homogenization_contributions(_homogenization_gradient_names.size()),
    _rotation_increment(declareProperty<RankTwoTensor>(_base_name + "rotation_increment"))
{
  // Couple old displacements only when the simulation is transient. With a Steady executioner
  // there is no "previous step", and the generalized midpoint rule treats the old state as
  // the undeformed reference (u_n = 0, grad u_n = 0, so F_n = I). The (1 - alpha) contribution
  // is then identically zero and we skip it in computeQpUnstabilizedDeformationGradient.
  if (_fe_problem.isTransient())
  {
    _disp_old = coupledValuesOld("displacements");
    _grad_disp_old = coupledGradientsOld("displacements");
  }

  // Setup eigenstrains
  for (auto i : make_range(_eigenstrain_names.size()))
  {
    _eigenstrains[i] = &getMaterialProperty<RankTwoTensor>(_eigenstrain_names[i]);
    _eigenstrains_old[i] = &getMaterialPropertyOld<RankTwoTensor>(_eigenstrain_names[i]);
  }

  // In the future maybe there is a reason to have more than one, but for now
  if (_homogenization_gradient_names.size() > 1)
    mooseError("ComputeLagrangianStrainBase cannot accommodate more than one "
               "homogenization gradient");

  // Setup homogenization contributions
  for (unsigned int i = 0; i < _homogenization_gradient_names.size(); i++)
    _homogenization_contributions[i] =
        &getMaterialProperty<RankTwoTensor>(_homogenization_gradient_names[i]);
}

template <class G>
void
ComputeLagrangianStrainBase<G>::initQpStatefulProperties()
{
  _total_strain[_qp].zero();
  _mechanical_strain[_qp].zero();
  _F[_qp].setToIdentity();
}

template <class G>
void
ComputeLagrangianStrainBase<G>::computeProperties()
{
  // Average the volumetric terms, if required
  computeDeformationGradient();

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    computeQpProperties();
}

template <class G>
void
ComputeLagrangianStrainBase<G>::computeQpProperties()
{
  // Add in the macroscale gradient contribution
  for (auto contribution : _homogenization_contributions)
    _F[_qp] += (*contribution)[_qp];

  // If the kernel is large deformation then we need the "actual"
  // kinematic quantities
  RankTwoTensor dL;
  usingTensorIndices(k_, l_, m_, n_);
  if (_large_kinematics)
  {
    _F_inv[_qp] = _F[_qp].inverse();
    _f_inv[_qp] = _F_old[_qp] * _F_inv[_qp];
    dL = RankTwoTensor::Identity() - _f_inv[_qp];

    // d(dL)_{kl}/dF_{mn} = f^{-1}_{km} * F^{-1}_{nl} for the linear approximation dL = I - f^{-1}.
    _d_spatial_velocity_increment_d_F[_qp] =
        _f_inv[_qp].template times<k_, m_, n_, l_>(_F_inv[_qp]);
  }
  // For small deformations we just provide the identity
  else
  {
    _F_inv[_qp] = RankTwoTensor::Identity();
    _f_inv[_qp] = RankTwoTensor::Identity();
    dL = _F[_qp] - _F_old[_qp];

    // d(dL)/dF = I^{(4)} when dL = F - F_old.
    _d_spatial_velocity_increment_d_F[_qp] = RankFourTensor::IdentityFour();
  }

  // dF/d(grad u_{n+1}) = alpha * I^{(4)} for the generalized midpoint rule
  // (alpha = 1.0 reduces to backward Euler).
  _d_F_d_grad_u[_qp] = _alpha * RankFourTensor::IdentityFour();

  computeQpIncrementalStrains(dL);
}

template <class G>
void
ComputeLagrangianStrainBase<G>::computeQpIncrementalStrains(const RankTwoTensor & dL)
{
  // Get the deformation increments
  _strain_increment[_qp] = (dL + dL.transpose()) / 2.0;
  _vorticity_increment[_qp] = (dL - dL.transpose()) / 2.0;
  // Full kinematic spatial velocity gradient increment, before any eigenstrain subtraction.
  // The objective-rate advection in ComputeLagrangianObjectiveStress consumes this.
  _spatial_velocity_increment[_qp] = dL;

  // Increment the total strain
  _total_strain[_qp] = _total_strain_old[_qp] + _strain_increment[_qp];

  // Get rid of the eigenstrains
  // Note we currently do not alter the deformation gradient -- this will be
  // needed in the future for a "complete" system
  subtractQpEigenstrainIncrement(_strain_increment[_qp]);

  // Increment the mechanical strain
  _mechanical_strain[_qp] = _mechanical_strain_old[_qp] + _strain_increment[_qp];

  // Faked rotation increment for ComputeStressBase materials
  _rotation_increment[_qp] = RankTwoTensor::Identity();
}

template <class G>
void
ComputeLagrangianStrainBase<G>::subtractQpEigenstrainIncrement(RankTwoTensor & strain)
{
  for (auto i : make_range(_eigenstrain_names.size()))
    strain -= (*_eigenstrains[i])[_qp] - (*_eigenstrains_old[i])[_qp];
}

template <class G>
void
ComputeLagrangianStrainBase<G>::computeQpUnstabilizedDeformationGradient()
{
  // Generalized midpoint: F^alpha_{n+1} = I + alpha * (grad u_{n+1}, u_{n+1})
  //                                         + (1 - alpha) * (grad u_n, u_n).
  // alpha = 1.0 reduces to backward Euler (no old contribution). With a Steady executioner
  // the old displacement is treated as identically zero (F_n = I), so we skip the old call
  // entirely - this lets a user run with alpha != 1 in steady mode as well.
  _F_ust[_qp].setToIdentity();
  const bool include_old = _alpha != 1.0 && _fe_problem.isTransient();
  for (auto component : make_range(_ndisp))
  {
    G::addGradOp(_F_ust[_qp],
                 component,
                 _alpha * (*_grad_disp[component])[_qp],
                 _alpha * (*_disp[component])[_qp],
                 _q_point[_qp]);
    if (include_old)
      G::addGradOp(_F_ust[_qp],
                   component,
                   (1.0 - _alpha) * (*_grad_disp_old[component])[_qp],
                   (1.0 - _alpha) * (*_disp_old[component])[_qp],
                   _q_point[_qp]);
  }
}

template <class G>
void
ComputeLagrangianStrainBase<G>::computeQpActualDeformationGradient()
{
  // The literal deformation gradient at n+1 (no alpha weighting, no F-bar). For alpha = 1
  // this is identical to _F_ust.
  _F_actual[_qp].setToIdentity();
  for (auto component : make_range(_ndisp))
    G::addGradOp(_F_actual[_qp],
                 component,
                 (*_grad_disp[component])[_qp],
                 (*_disp[component])[_qp],
                 _q_point[_qp]);
}

template <class G>
void
ComputeLagrangianStrainBase<G>::computeDeformationGradient()
{
  // First calculate the unstabilized deformation gradient at each qp
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    computeQpUnstabilizedDeformationGradient();
    computeQpActualDeformationGradient();
    _F[_qp] = _F_ust[_qp];
  }

  usingTensorIndices(i_, j_, k_, l_);
  const auto I2 = RankTwoTensor::Identity();

  // If stabilization is on do the volumetric correction
  if (_stabilize_strain)
  {
    const auto F_avg = StabilizationUtils::elementAverage(
        [this](unsigned int qp) { return _F_ust[qp]; }, _JxW, _coord);
    // All quadrature points have the same F_avg
    _F_avg.set().setAllValues(F_avg);
    // Make the appropriate modification, depending on small or large
    // deformations
    for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    {
      if (_large_kinematics)
      {
        // Multiplicative F-bar: F_stab = (det F_avg / det F_ust)^{1/3} * F_ust.
        // Chain-rule partials:
        //   dF_stab/dF_ust = gamma * I^(4) - (gamma/3) * F_ust ⊗ F_ust^{-T}
        //   dF_stab/dF_avg = (gamma/3) * F_ust ⊗ F_avg^{-T}
        const Real gamma = std::pow(F_avg.det() / _F[_qp].det(), 1.0 / 3.0);
        const auto Fust_invT = _F_ust[_qp].inverse().transpose();
        const auto Favg_invT = F_avg.inverse().transpose();
        _d_F_stab_d_F_ust[_qp] = gamma * RankFourTensor::IdentityFour() -
                                 (gamma / 3.0) * _F_ust[_qp].template times<i_, j_, k_, l_>(Fust_invT);
        _d_F_stab_d_F_avg[_qp] =
            (gamma / 3.0) * _F_ust[_qp].template times<i_, j_, k_, l_>(Favg_invT);
        _F[_qp] *= gamma;
      }
      else
      {
        // Additive (trace) F-bar: F_stab = F_ust + (tr(F_avg - F_ust)/3) * I.
        // dF_stab/dF_ust = I^(4) - (1/3) * I2 ⊗ I2  (each diagonal component pulled out).
        // dF_stab/dF_avg = (1/3) * I2 ⊗ I2.
        const auto outer = I2.template times<i_, j_, k_, l_>(I2);
        _d_F_stab_d_F_ust[_qp] = RankFourTensor::IdentityFour() - (1.0 / 3.0) * outer;
        _d_F_stab_d_F_avg[_qp] = (1.0 / 3.0) * outer;
        _F[_qp] += (F_avg.trace() - _F[_qp].trace()) * I2 / 3.0;
      }
    }
  }
  else
  {
    // F-bar off: dF_stab/dF_ust = I^(4), dF_stab/dF_avg = 0.
    for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    {
      _d_F_stab_d_F_ust[_qp] = RankFourTensor::IdentityFour();
      _d_F_stab_d_F_avg[_qp].zero();
    }
  }
}

template class ComputeLagrangianStrainBase<GradientOperatorCartesian>;
template class ComputeLagrangianStrainBase<GradientOperatorAxisymmetricCylindrical>;
template class ComputeLagrangianStrainBase<GradientOperatorCentrosymmetricSpherical>;
