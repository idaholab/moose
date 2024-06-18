//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeIncrementalStrain.h"
#include "Assembly.h"
#include "libmesh/quadrature.h"

registerMooseObject("SolidMechanicsApp", ComputeIncrementalStrain);
registerMooseObjectRenamed("SolidMechanicsApp",
                           ComputeIncrementalSmallStrain,
                           "04/01/2025 00:00",
                           ComputeIncrementalStrain);

InputParameters
ComputeIncrementalStrain::validParams()
{
  InputParameters params = ComputeIncrementalStrainBase::validParams();
  params.addClassDescription(
      "Compute a strain increment and rotation increment for small strains.");
  return params;
}

ComputeIncrementalStrain::ComputeIncrementalStrain(const InputParameters & parameters)
  : ComputeIncrementalStrainBase(parameters)
{
}

void
ComputeIncrementalStrain::computeProperties()
{
  Real volumetric_strain = 0.0;
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    RankTwoTensor total_strain_increment;
    computeTotalStrainIncrement(total_strain_increment);

    _strain_increment[_qp] = total_strain_increment;

    if (_volumetric_locking_correction)
      volumetric_strain += total_strain_increment.trace() * _JxW[_qp] * _coord[_qp];
  }
  if (_volumetric_locking_correction)
    volumetric_strain /= _current_elem_volume;

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    Real trace = _strain_increment[_qp].trace();
    if (_volumetric_locking_correction)
    {
      _strain_increment[_qp](0, 0) += (volumetric_strain - trace) / 3.0;
      _strain_increment[_qp](1, 1) += (volumetric_strain - trace) / 3.0;
      _strain_increment[_qp](2, 2) += (volumetric_strain - trace) / 3.0;
    }

    _total_strain[_qp] = _total_strain_old[_qp] + _strain_increment[_qp];

    // Remove the Eigen strain increment
    subtractEigenstrainIncrementFromStrain(_strain_increment[_qp]);

    // strain rate
    if (_dt > 0)
      _strain_rate[_qp] = _strain_increment[_qp] / _dt;
    else
      _strain_rate[_qp].zero();

    // Update strain in intermediate configuration: rotations are not needed
    _mechanical_strain[_qp] = _mechanical_strain_old[_qp] + _strain_increment[_qp];

    // incremental small strain does not include rotation
    _rotation_increment[_qp].setToIdentity();
  }
}

void
ComputeIncrementalStrain::computeTotalStrainIncrement(RankTwoTensor & total_strain_increment)
{
  // Deformation gradient
  auto A = RankTwoTensor::initializeFromRows(
      (*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp], (*_grad_disp[2])[_qp]);

  // Old Deformation gradient
  auto Fbar = RankTwoTensor ::initializeFromRows(
      (*_grad_disp_old[0])[_qp], (*_grad_disp_old[1])[_qp], (*_grad_disp_old[2])[_qp]);

  _deformation_gradient[_qp] = A;
  _deformation_gradient[_qp].addIa(1.0);

  A -= Fbar; // A = grad_disp - grad_disp_old

  total_strain_increment = 0.5 * (A + A.transpose());
}
