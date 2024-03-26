//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeIncrementalStrainBase.h"
#include "MooseMesh.h"

InputParameters
ComputeIncrementalStrainBase::validParams()
{
  InputParameters params = ComputeStrainBase::validParams();
  return params;
}

ComputeIncrementalStrainBase::ComputeIncrementalStrainBase(const InputParameters & parameters)
  : ComputeStrainBase(parameters),
    _grad_disp_old(3),
    _strain_rate(declareProperty<RankTwoTensor>(_base_name + "strain_rate")),
    _strain_increment(declareProperty<RankTwoTensor>(_base_name + "strain_increment")),
    _rotation_increment(declareProperty<RankTwoTensor>(_base_name + "rotation_increment")),
    _deformation_gradient(declareProperty<RankTwoTensor>(_base_name + "deformation_gradient")),
    _mechanical_strain_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "mechanical_strain")),
    _total_strain_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "total_strain")),
    _eigenstrains_old(_eigenstrain_names.size())
{
  for (unsigned int i = 0; i < _eigenstrains_old.size(); ++i)
    _eigenstrains_old[i] = &getMaterialPropertyOld<RankTwoTensor>(_eigenstrain_names[i]);
}

void
ComputeIncrementalStrainBase::initialSetup()
{
  ComputeStrainBase::initialSetup();
  for (unsigned int i = 0; i < 3; ++i)
  {
    if (_fe_problem.isTransient() && i < _ndisp)
      _grad_disp_old[i] = &coupledGradientOld("displacements", i);
    else
      _grad_disp_old[i] = &_grad_zero;
  }
}

void
ComputeIncrementalStrainBase::initQpStatefulProperties()
{
  _mechanical_strain[_qp].zero();
  _total_strain[_qp].zero();
  _deformation_gradient[_qp].setToIdentity();
  _rotation_increment[_qp].setToIdentity();
}

void
ComputeIncrementalStrainBase::subtractEigenstrainIncrementFromStrain(RankTwoTensor & strain)
{
  for (unsigned int i = 0; i < _eigenstrains.size(); ++i)
  {
    strain -= (*_eigenstrains[i])[_qp];
    strain += (*_eigenstrains_old[i])[_qp];
  }
}
