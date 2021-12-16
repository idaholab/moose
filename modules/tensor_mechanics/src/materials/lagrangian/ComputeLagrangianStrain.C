//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLagrangianStrain.h"

registerMooseObject("TensorMechanicsApp", ComputeLagrangianStrain);

InputParameters
ComputeLagrangianStrain::validParams()
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
      "List of homogenization gradients to add to the "
      "displacement gradient");

  params.addParam<std::string>("base_name", "Material property base name");

  // We rely on this *not* having use_displaced mesh on
  params.suppressParameter<bool>("use_displaced_mesh");

  return params;
}

ComputeLagrangianStrain::ComputeLagrangianStrain(const InputParameters & parameters)
  : Material(parameters),
    _ndisp(coupledComponents("displacements")),
    _disp(3),
    _grad_disp(3),
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
    _def_grad(declareProperty<RankTwoTensor>(_base_name + "deformation_gradient")),
    _def_grad_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "deformation_gradient")),
    _unstabilized_def_grad(
        declareProperty<RankTwoTensor>(_base_name + "unstabilized_deformation_gradient")),
    _avg_def_grad(declareProperty<RankTwoTensor>(_base_name + "avg_deformation_gradient")),
    _inv_inv_inc_def_grad(declareProperty<RankTwoTensor>(_base_name + "inv_inc_def_grad")),
    _inv_def_grad(declareProperty<RankTwoTensor>(_base_name + "inv_def_grad")),
    _detJ(declareProperty<Real>(_base_name + "detJ")),
    _homogenization_gradient_names(
        getParam<std::vector<MaterialPropertyName>>("homogenization_gradient_names")),
    _homogenization_contributions(_homogenization_gradient_names.size())
{
  // Setup eigenstrains
  for (unsigned int i = 0; i < _eigenstrain_names.size(); i++)
  {
    _eigenstrains[i] = &getMaterialProperty<RankTwoTensor>(_eigenstrain_names[i]);
    _eigenstrains_old[i] = &getMaterialPropertyOld<RankTwoTensor>(_eigenstrain_names[i]);
  }

  // In the future maybe there is a reason to have more than one, but for now
  if (_homogenization_gradient_names.size() > 1)
    mooseError("ComputeLagrangianStrain cannot accommodate more than one "
               "homogenization gradient");

  // Setup homogenization contributions
  for (unsigned int i = 0; i < _homogenization_gradient_names.size(); i++)
    _homogenization_contributions[i] =
        &getMaterialProperty<RankTwoTensor>(_homogenization_gradient_names[i]);
}

void
ComputeLagrangianStrain::initialSetup()
{
  // Setup displacements/zeros
  for (unsigned int i = 0; i < 3; i++)
  {
    // Actual displacement
    if (i < _ndisp)
    {
      _disp[i] = &coupledValue("displacements", i);
      _grad_disp[i] = &coupledGradient("displacements", i);
    }
    // Zero
    else
    {
      _disp[i] = &_zero;
      _grad_disp[i] = &_grad_zero;
    }
  }
}

void
ComputeLagrangianStrain::initQpStatefulProperties()
{
  _total_strain[_qp].zero();
  _mechanical_strain[_qp].zero();
  _def_grad[_qp].setToIdentity();
}

void
ComputeLagrangianStrain::computeProperties()
{
  // Average the volumetric terms, if required
  calculateDeformationGradient();

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    computeQpProperties();
}

void
ComputeLagrangianStrain::computeQpProperties()
{
  // Add in the macroscale gradient contribution
  for (auto contribution : _homogenization_contributions)
    _def_grad[_qp] += (*contribution)[_qp];

  // If the kernel is large deformation then we need the "actual"
  // kinematic quantities
  RankTwoTensor L;
  if (_large_kinematics)
  {
    _inv_def_grad[_qp] = _def_grad[_qp].inverse();
    _detJ[_qp] = _def_grad[_qp].det();
    _inv_inv_inc_def_grad[_qp] = _def_grad_old[_qp] * _inv_def_grad[_qp];
    L = RankTwoTensor::Identity() - _inv_inv_inc_def_grad[_qp];
  }
  // For small deformations we just provide the identity
  else
  {
    _inv_def_grad[_qp] = RankTwoTensor::Identity();
    _detJ[_qp] = 1.0;
    _inv_inv_inc_def_grad[_qp] = RankTwoTensor::Identity();
    L = _def_grad[_qp] - _def_grad_old[_qp];
  }

  calculateIncrementalStrains(L);
}

void
ComputeLagrangianStrain::calculateIncrementalStrains(const RankTwoTensor & L)
{
  // Get the deformation increments
  _strain_increment[_qp] = (L + L.transpose()) / 2.0;

  // Increment the total strain
  _total_strain[_qp] = _total_strain_old[_qp] + _strain_increment[_qp];

  // Get rid of the eigenstrains
  // Note we currently do not alter the deformation gradient -- this will be
  // needed in the future for a "complete" system
  subtractEigenstrainIncrement(_strain_increment[_qp]);

  // Increment the mechanical strain
  _mechanical_strain[_qp] = _mechanical_strain_old[_qp] + _strain_increment[_qp];
}

void
ComputeLagrangianStrain::subtractEigenstrainIncrement(RankTwoTensor & strain)
{
  for (size_t i = 0; i < _eigenstrain_names.size(); i++)
  {
    strain -= (*_eigenstrains[i])[_qp];
    strain += (*_eigenstrains_old[i])[_qp];
  }
}

void
ComputeLagrangianStrain::calculateDeformationGradient()
{
  // First calculate the base deformation gradient at each qp
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    _unstabilized_def_grad[_qp] =
        (RankTwoTensor::Identity() + RankTwoTensor::initializeFromRows((*_grad_disp[0])[_qp],
                                                                       (*_grad_disp[1])[_qp],
                                                                       (*_grad_disp[2])[_qp]));

  // If stabilization is on do the volumetric correction
  if (_stabilize_strain)
  {
    // Average the deformation gradient
    RankTwoTensor F0;
    F0.zero();
    Real cv = 0.0;
    for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    {
      cv += _JxW[_qp] * _coord[_qp];
      F0 += _JxW[_qp] * _coord[_qp] * _unstabilized_def_grad[_qp];
    }
    F0 /= cv;
    // Make the appropriate modification, depending on small or large
    // deformations
    for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    {
      _avg_def_grad[_qp] = F0;
      if (_large_kinematics)
      {
        _def_grad[_qp] =
            std::pow(_avg_def_grad[_qp].det() / _unstabilized_def_grad[_qp].det(), 1.0 / 3.0) *
            _unstabilized_def_grad[_qp];
      }
      else
        _def_grad[_qp] = _unstabilized_def_grad[_qp] +
                         (_avg_def_grad[_qp].trace() - _unstabilized_def_grad[_qp].trace()) *
                             RankTwoTensor::Identity() / 3.0;
    }
  }
  // If not stabilized just copy over
  else
  {
    for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    {
      _avg_def_grad[_qp] = _unstabilized_def_grad[_qp];
      _def_grad[_qp] = _unstabilized_def_grad[_qp];
    }
  }
}
