//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeIncrementalSmallStrain.h"
#include "libmesh/quadrature.h"

registerADMooseObject("TensorMechanicsApp", ADComputeIncrementalSmallStrain);

defineADValidParams(ADComputeIncrementalSmallStrain,
                    ADComputeIncrementalStrainBase,
                    params.addClassDescription(
                        "Compute a strain increment and rotation increment for small strains."););

template <ComputeStage compute_stage>
ADComputeIncrementalSmallStrain<compute_stage>::ADComputeIncrementalSmallStrain(
    const InputParameters & parameters)
  : ADComputeIncrementalStrainBase<compute_stage>(parameters)
{
}

template <ComputeStage compute_stage>
void
ADComputeIncrementalSmallStrain<compute_stage>::computeProperties()
{
  ADReal volumetric_strain = 0.0;
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    ADRankTwoTensor total_strain_increment;
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
      _strain_increment[_qp](0, 0) += correction;
      _strain_increment[_qp](1, 1) += correction;
      _strain_increment[_qp](2, 2) += correction;
    }

    _total_strain[_qp] = _strain_increment[_qp] + _total_strain_old[_qp];

    // Remove the Eigen strain increment
    subtractEigenstrainIncrementFromStrain(_strain_increment[_qp]);

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

  copyDualNumbersToValues();
}

template <ComputeStage compute_stage>
void
ADComputeIncrementalSmallStrain<compute_stage>::computeTotalStrainIncrement(
    ADRankTwoTensor & total_strain_increment)
{
  // Deformation gradient
  ADRankTwoTensor A(
      (*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp], (*_grad_disp[2])[_qp]); // Deformation gradient
  RankTwoTensor Fbar((*_grad_disp_old[0])[_qp],
                     (*_grad_disp_old[1])[_qp],
                     (*_grad_disp_old[2])[_qp]); // Old Deformation gradient

  A -= Fbar; // A = grad_disp - grad_disp_old

  total_strain_increment = 0.5 * (A + A.transpose());
}

// explicit instantiation is required for AD base classes
adBaseClass(ADComputeIncrementalSmallStrain);
