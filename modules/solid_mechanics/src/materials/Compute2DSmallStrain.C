//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Compute2DSmallStrain.h"

#include "libmesh/quadrature.h"

InputParameters
Compute2DSmallStrain::validParams()
{
  InputParameters params = ComputeSmallStrain::validParams();
  params.addClassDescription("Compute a small strain in a plane strain configuration.");

  MooseEnum outOfPlaneDirection("x y z", "z");
  params.addParam<MooseEnum>(
      "out_of_plane_direction", outOfPlaneDirection, "The direction of the out-of-plane strain.");
  return params;
}

Compute2DSmallStrain::Compute2DSmallStrain(const InputParameters & parameters)
  : ComputeSmallStrain(parameters),
    _out_of_plane_direction(getParam<MooseEnum>("out_of_plane_direction"))
{
}

void
Compute2DSmallStrain::initialSetup()
{
  displacementIntegrityCheck();
  for (unsigned int i = 0; i < 3; ++i)
  {
    if (_out_of_plane_direction == i)
    {
      _disp[i] = &_zero;
      _grad_disp[i] = &_grad_zero;
    }
    else
    {
      _disp[i] = &coupledValue("displacements", i);
      _grad_disp[i] = &coupledGradient("displacements", i);
    }
  }
}

void
Compute2DSmallStrain::computeProperties()
{
  const auto o0 = _out_of_plane_direction;
  const auto o1 = (_out_of_plane_direction + 1) % 3;
  const auto o2 = (_out_of_plane_direction + 2) % 3;

  Real volumetric_strain = 0.0;
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    _total_strain[_qp](o0, o0) = computeOutOfPlaneStrain();
    _total_strain[_qp](o1, o1) = (*_grad_disp[o1])[_qp](o1);
    _total_strain[_qp](o2, o2) = (*_grad_disp[o2])[_qp](o2);
    _total_strain[_qp](o1, o2) = ((*_grad_disp[o1])[_qp](o2) + (*_grad_disp[o2])[_qp](o1)) / 2.0;
    _total_strain[_qp](o2, o1) = _total_strain[_qp](o1, o2); // force the symmetrical strain tensor

    if (_volumetric_locking_correction)
      volumetric_strain += _total_strain[_qp].trace() * _JxW[_qp] * _coord[_qp];
  }

  if (_volumetric_locking_correction)
    volumetric_strain /= _current_elem_volume;

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    if (_volumetric_locking_correction)
    {
      const Real correction = (volumetric_strain - _total_strain[_qp].trace()) / 3.0;
      _total_strain[_qp](0, 0) += correction;
      _total_strain[_qp](1, 1) += correction;
      _total_strain[_qp](2, 2) += correction;
    }

    _mechanical_strain[_qp] = _total_strain[_qp];

    // Remove the eigenstrains
    for (const auto es : _eigenstrains)
      _mechanical_strain[_qp] -= (*es)[_qp];
  }
}

void
Compute2DSmallStrain::displacementIntegrityCheck()
{
  if (_out_of_plane_direction != 2 && _ndisp != 3)
    mooseError("For 2D simulations where the out-of-plane direction is x or y the number of "
               "supplied displacements must be three.");
  else if (_out_of_plane_direction == 2 && _ndisp != 2)
    mooseError("For 2D simulations where the out-of-plane direction is z the number of supplied "
               "displacements must be two.");
}
