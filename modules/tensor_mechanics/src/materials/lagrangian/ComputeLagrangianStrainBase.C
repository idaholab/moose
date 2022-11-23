//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
  params.addParam<std::vector<MaterialPropertyName>>("eigenstrain_names",
                                                     "List of eigenstrains to account for");
  params.addParam<std::vector<MaterialPropertyName>>(
      "homogenization_gradient_names",
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
    _eigenstrain_names(getParam<std::vector<MaterialPropertyName>>("eigenstrain_names")),
    _eigenstrains(_eigenstrain_names.size()),
    _eigenstrains_old(_eigenstrain_names.size()),
    _total_strain(declareProperty<RankTwoTensor>(_base_name + "total_strain")),
    _total_strain_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "total_strain")),
    _mechanical_strain(declareProperty<RankTwoTensor>(_base_name + "mechanical_strain")),
    _mechanical_strain_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "mechanical_strain")),
    _strain_increment(declareProperty<RankTwoTensor>(_base_name + "strain_increment")),
    _vorticity_increment(declareProperty<RankTwoTensor>(_base_name + "vorticity_increment")),
    _F_ust(declareProperty<RankTwoTensor>(_base_name + "unstabilized_deformation_gradient")),
    _F_avg(declareProperty<RankTwoTensor>(_base_name + "average_deformation_gradient")),
    _F(declareProperty<RankTwoTensor>(_base_name + "deformation_gradient")),
    _F_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "deformation_gradient")),
    _F_inv(declareProperty<RankTwoTensor>(_base_name + "inverse_deformation_gradient")),
    _f_inv(declareProperty<RankTwoTensor>(_base_name + "inverse_incremental_deformation_gradient")),
    _homogenization_gradient_names(
        getParam<std::vector<MaterialPropertyName>>("homogenization_gradient_names")),
    _homogenization_contributions(_homogenization_gradient_names.size()),
    _rotation_increment(declareProperty<RankTwoTensor>(_base_name + "rotation_increment"))
{
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
  if (_large_kinematics)
  {
    _F_inv[_qp] = _F[_qp].inverse();
    _f_inv[_qp] = _F_old[_qp] * _F_inv[_qp];
    dL = RankTwoTensor::Identity() - _f_inv[_qp];
  }
  // For small deformations we just provide the identity
  else
  {
    _F_inv[_qp] = RankTwoTensor::Identity();
    _f_inv[_qp] = RankTwoTensor::Identity();
    dL = _F[_qp] - _F_old[_qp];
  }

  computeQpIncrementalStrains(dL);
}

template <class G>
void
ComputeLagrangianStrainBase<G>::computeQpIncrementalStrains(const RankTwoTensor & dL)
{
  // Get the deformation increments
  _strain_increment[_qp] = (dL + dL.transpose()) / 2.0;
  _vorticity_increment[_qp] = (dL - dL.transpose()) / 2.0;

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
  _F_ust[_qp].setToIdentity();
  for (auto component : make_range(_ndisp))
    G::addGradOp(_F_ust[_qp],
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
    _F[_qp] = _F_ust[_qp];
  }

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
        _F[_qp] *= std::pow(F_avg.det() / _F[_qp].det(), 1.0 / 3.0);
      else
        _F[_qp] += (F_avg.trace() - _F[_qp].trace()) * RankTwoTensor::Identity() / 3.0;
    }
  }
}

template class ComputeLagrangianStrainBase<GradientOperatorCartesian>;
template class ComputeLagrangianStrainBase<GradientOperatorAxisymmetricCylindrical>;
template class ComputeLagrangianStrainBase<GradientOperatorCentrosymmetricSpherical>;
