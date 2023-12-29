//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeSmallStrain.h"
#include "Assembly.h"
#include "libmesh/quadrature.h"

registerMooseObject("TensorMechanicsApp", ComputeSmallStrain);

InputParameters
ComputeSmallStrain::validParams()
{
  InputParameters params = ComputeStrainBase::validParams();
  params.addClassDescription("Compute a small strain.");
  return params;
}

ComputeSmallStrain::ComputeSmallStrain(const InputParameters & parameters)
  : ComputeStrainBase(parameters)
{
}

void
ComputeSmallStrain::computeProperties()
{
  Real volumetric_strain = 0.0;
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    // strain = (grad_disp + grad_disp^T)/2
    const auto grad_tensor = RankTwoTensor ::initializeFromRows(
        (*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp], (*_grad_disp[2])[_qp]);

    _total_strain[_qp] = (grad_tensor + grad_tensor.transpose()) / 2.0;

    if (_volumetric_locking_correction)
      volumetric_strain += _total_strain[_qp].trace() * _JxW[_qp] * _coord[_qp];
  }

  if (_volumetric_locking_correction)
    volumetric_strain /= _current_elem_volume;

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    if (_volumetric_locking_correction)
    {
      Real trace = _total_strain[_qp].trace();
      _total_strain[_qp](0, 0) += (volumetric_strain - trace) / 3.0;
      _total_strain[_qp](1, 1) += (volumetric_strain - trace) / 3.0;
      _total_strain[_qp](2, 2) += (volumetric_strain - trace) / 3.0;
    }

    if (_global_strain)
      _total_strain[_qp] += (*_global_strain)[_qp];

    _mechanical_strain[_qp] = _total_strain[_qp];

    // Remove the Eigen strain
    for (auto es : _eigenstrains)
      _mechanical_strain[_qp] -= (*es)[_qp];
  }
}
