/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "Compute2DSmallStrain.h"

// libmesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<Compute2DSmallStrain>()
{
  InputParameters params = validParams<ComputeSmallStrain>();
  params.addClassDescription("Compute a small strain in a plane strain configuration.");
  return params;
}

Compute2DSmallStrain::Compute2DSmallStrain(const InputParameters & parameters)
  : ComputeSmallStrain(parameters)
{
}

void
Compute2DSmallStrain::computeProperties()
{
  Real volumetric_strain = 0.0;
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    _total_strain[_qp](0, 0) = (*_grad_disp[0])[_qp](0);
    _total_strain[_qp](1, 1) = (*_grad_disp[1])[_qp](1);
    _total_strain[_qp](0, 1) = ((*_grad_disp[0])[_qp](1) + (*_grad_disp[1])[_qp](0)) / 2.0;
    _total_strain[_qp](1, 0) = _total_strain[_qp](0, 1); // force the symmetrical strain tensor
    _total_strain[_qp](2, 2) = computeStrainZZ();

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

    _mechanical_strain[_qp] = _total_strain[_qp];

    // Remove the eigenstrains
    for (auto es : _eigenstrains)
      _mechanical_strain[_qp] -= (*es)[_qp];
  }
}
