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
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "SymmetricRankTwoTensor.h"
#include "SymmetricRankFourTensor.h"

template <typename R2>
InputParameters
ADComputeIncrementalStrainBaseTempl<R2>::validParams()
{
  InputParameters params = ADComputeStrainBase::validParams();
  return params;
}

template <typename R2>
ADComputeIncrementalStrainBaseTempl<R2>::ADComputeIncrementalStrainBaseTempl(
    const InputParameters & parameters)
  : ADComputeStrainBaseTempl<R2>(parameters),
    _grad_disp_old(3),
    _strain_rate(this->template declareADProperty<R2>(_base_name + "strain_rate")),
    _strain_increment(this->template declareADProperty<R2>(_base_name + "strain_increment")),
    _rotation_increment(
        this->template declareADProperty<RankTwoTensor>(_base_name + "rotation_increment")),
    _mechanical_strain_old(
        this->template getMaterialPropertyOld<R2>(_base_name + "mechanical_strain")),
    _total_strain_old(this->template getMaterialPropertyOld<R2>(_base_name + "total_strain")),
    _eigenstrains_old(_eigenstrain_names.size())
{
  for (unsigned int i = 0; i < _eigenstrains_old.size(); ++i)
    _eigenstrains_old[i] = &this->template getMaterialPropertyOld<R2>(_eigenstrain_names[i]);
}

template <typename R2>
void
ADComputeIncrementalStrainBaseTempl<R2>::initialSetup()
{
  ADComputeStrainBaseTempl<R2>::initialSetup();
  for (unsigned int i = 0; i < 3; ++i)
  {
    if (this->_fe_problem.isTransient() && i < _ndisp)
      _grad_disp_old[i] = &this->coupledGradientOld("displacements", i);
    else
      _grad_disp_old[i] = &_grad_zero;
  }
}

template <typename R2>
void
ADComputeIncrementalStrainBaseTempl<R2>::initQpStatefulProperties()
{
  _mechanical_strain[_qp].zero();
  _total_strain[_qp].zero();
}

template <typename R2>
void
ADComputeIncrementalStrainBaseTempl<R2>::subtractEigenstrainIncrementFromStrain(ADR2 & strain)
{
  for (unsigned int i = 0; i < _eigenstrains.size(); ++i)
  {
    strain -= (*_eigenstrains[i])[_qp];
    strain += (*_eigenstrains_old[i])[_qp];
  }
}

template class ADComputeIncrementalStrainBaseTempl<RankTwoTensor>;
template class ADComputeIncrementalStrainBaseTempl<SymmetricRankTwoTensor>;
