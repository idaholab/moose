//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeIncrementalSmallStrain.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "SymmetricRankTwoTensor.h"
#include "SymmetricRankFourTensor.h"

#include "libmesh/quadrature.h"

registerMooseObject("TensorMechanicsApp", ADComputeIncrementalSmallStrain);
registerMooseObject("TensorMechanicsApp", ADSymmetricIncrementalSmallStrain);

template <typename R2>
InputParameters
ADComputeIncrementalSmallStrainTempl<R2>::validParams()
{
  InputParameters params = ADComputeIncrementalStrainBase::validParams();
  params.addClassDescription(
      "Compute a strain increment and rotation increment for small strains.");
  return params;
}

template <typename R2>
ADComputeIncrementalSmallStrainTempl<R2>::ADComputeIncrementalSmallStrainTempl(
    const InputParameters & parameters)
  : ADComputeIncrementalStrainBaseTempl<R2>(parameters)
{
}

template <typename R2>
void
ADComputeIncrementalSmallStrainTempl<R2>::computeProperties()
{
  ADReal volumetric_strain = 0.0;
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    ADR2 total_strain_increment;
    computeTotalStrainIncrement(total_strain_increment);

    _strain_increment[_qp] = total_strain_increment;

    if (_volumetric_locking_correction)
      volumetric_strain += total_strain_increment.trace() * _JxW[_qp] * _coord[_qp];
  }

  if (_volumetric_locking_correction)
    volumetric_strain /= _current_elem_volume;

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    if (_volumetric_locking_correction)
    {
      const auto correction = (volumetric_strain - _strain_increment[_qp].trace()) / 3.0;
      _strain_increment[_qp].addIa(correction);
    }

    _total_strain[_qp] = _strain_increment[_qp] + _total_strain_old[_qp];

    // Remove the Eigen strain increment
    this->subtractEigenstrainIncrementFromStrain(_strain_increment[_qp]);

    // strain rate
    if (_dt > 0)
      _strain_rate[_qp] = _strain_increment[_qp] / _dt;
    else
      _strain_rate[_qp].zero();

    // Update strain in intermediate configuration: rotations are not needed
    _mechanical_strain[_qp] = _strain_increment[_qp] + _mechanical_strain_old[_qp];

    // incremental small strain does not include rotation
    _rotation_increment[_qp].setToIdentity();
  }
}

template <typename R2>
void
ADComputeIncrementalSmallStrainTempl<R2>::computeTotalStrainIncrement(ADR2 & total_strain_increment)
{
  // Deformation gradient (symmetrized)
  const auto A = ADR2::initializeSymmetric(
      (*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp], (*_grad_disp[2])[_qp]);
  // Old Deformation gradient (symmetrized)
  const auto Fbar = ADR2::initializeSymmetric(
      (*_grad_disp_old[0])[_qp], (*_grad_disp_old[1])[_qp], (*_grad_disp_old[2])[_qp]);

  total_strain_increment = A - Fbar;
}

template class ADComputeIncrementalSmallStrainTempl<RankTwoTensor>;
template class ADComputeIncrementalSmallStrainTempl<SymmetricRankTwoTensor>;
