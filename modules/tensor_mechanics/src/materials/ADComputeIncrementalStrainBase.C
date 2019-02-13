//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeIncrementalStrainBase.h"
#include "MooseMesh.h"

defineADValidParams(ADComputeIncrementalStrainBase, ADComputeStrainBase, );

template <ComputeStage compute_stage>
ADComputeIncrementalStrainBase<compute_stage>::ADComputeIncrementalStrainBase(
    const InputParameters & parameters)
  : ADComputeStrainBase<compute_stage>(parameters),
    _grad_disp_old(3),
    _strain_rate(adDeclareADProperty<RankTwoTensor>(_base_name + "strain_rate")),
    _strain_increment(adDeclareADProperty<RankTwoTensor>(_base_name + "strain_increment")),
    _rotation_increment(adDeclareADProperty<RankTwoTensor>(_base_name + "rotation_increment")),
    _mechanical_strain_old(
        adGetMaterialPropertyOld<RankTwoTensor>(_base_name + "mechanical_strain")),
    _total_strain_old(adGetMaterialPropertyOld<RankTwoTensor>(_base_name + "total_strain")),
    _eigenstrains_old(_eigenstrain_names.size())
{
  for (unsigned int i = 0; i < _eigenstrains_old.size(); ++i)
    _eigenstrains_old[i] = &adGetMaterialPropertyOld<RankTwoTensor>(_eigenstrain_names[i]);
}

template <ComputeStage compute_stage>
void
ADComputeIncrementalStrainBase<compute_stage>::initialSetup()
{
  ADComputeStrainBase<compute_stage>::initialSetup();
  for (unsigned int i = 0; i < 3; ++i)
  {
    if (_fe_problem.isTransient() && i < _ndisp)
      _grad_disp_old[i] = &coupledGradientOld("displacements", i);
    else
      _grad_disp_old[i] = &_grad_zero;
  }
}

template <ComputeStage compute_stage>
void
ADComputeIncrementalStrainBase<compute_stage>::initQpStatefulProperties()
{
  _mechanical_strain[_qp].zero();
  _total_strain[_qp].zero();
}

template <ComputeStage compute_stage>
void
ADComputeIncrementalStrainBase<compute_stage>::subtractEigenstrainIncrementFromStrain(
    ADRankTwoTensor & strain)
{
  for (unsigned int i = 0; i < _eigenstrains.size(); ++i)
  {
    strain -= (*_eigenstrains[i])[_qp];
    strain += (*_eigenstrains_old[i])[_qp];
  }
}

// explicit instantiation is required for AD base classes
adBaseClass(ADComputeIncrementalStrainBase);
