//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCompute2DFiniteStrain.h"

#include "libmesh/quadrature.h"

InputParameters
ADCompute2DFiniteStrain::validParams()
{
  InputParameters params = ADComputeFiniteStrain::validParams();
  params.addClassDescription(
      "Compute a strain increment and rotation increment for finite strains in 2D geometries.");

  MooseEnum outOfPlaneDirection("x y z", "z");
  params.addParam<MooseEnum>(
      "out_of_plane_direction", outOfPlaneDirection, "The direction of the out-of-plane strain.");
  return params;
}

ADCompute2DFiniteStrain::ADCompute2DFiniteStrain(const InputParameters & parameters)
  : ADComputeFiniteStrain(parameters),
    _out_of_plane_direction(getParam<MooseEnum>("out_of_plane_direction"))
{
}

void
ADCompute2DFiniteStrain::initialSetup()
{
  for (unsigned int i = 0; i < 3; ++i)
  {
    if (_out_of_plane_direction == i)
    {
      _disp[i] = &_ad_zero;
      _grad_disp[i] = &_ad_grad_zero;
    }
    else
    {
      _disp[i] = &adCoupledValue("displacements", i);
      _grad_disp[i] = &adCoupledGradient("displacements", i);
    }

    if (_fe_problem.isTransient() && i != _out_of_plane_direction)
      _grad_disp_old[i] = &coupledGradientOld("displacements", i);
    else
      _grad_disp_old[i] = &_grad_zero;
  }
}

void
ADCompute2DFiniteStrain::computeProperties()
{
  ADRankTwoTensor ave_Fhat;

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    // Deformation gradient calculation for 2D problems
    auto A = ADRankTwoTensor::initializeFromRows(
        (*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp], (*_grad_disp[2])[_qp]);

    // Old Deformation gradient
    auto Fbar = RankTwoTensor::initializeFromRows(
        (*_grad_disp_old[0])[_qp], (*_grad_disp_old[1])[_qp], (*_grad_disp_old[2])[_qp]);

    // Compute the displacement gradient for the out of plane direction for plane strain,
    // generalized plane strain, or axisymmetric problems
    A(_out_of_plane_direction, _out_of_plane_direction) = computeOutOfPlaneGradDisp();
    Fbar(_out_of_plane_direction, _out_of_plane_direction) = computeOutOfPlaneGradDispOld();

    A -= Fbar; // very nearly A = gradU - gradUold

    Fbar.addIa(1.0); // Fbar = ( I + gradUold)

    // Incremental deformation gradient _Fhat = I + A Fbar^-1
    _Fhat[_qp] = A * Fbar.inverse();
    _Fhat[_qp].addIa(1.0);

    // Calculate average _Fhat for volumetric locking correction
    if (_volumetric_locking_correction)
      ave_Fhat += _Fhat[_qp] * _JxW[_qp] * _coord[_qp];
  }

  if (_volumetric_locking_correction)
    ave_Fhat /= _current_elem_volume;

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    // Finalize volumetric locking correction
    if (_volumetric_locking_correction)
      _Fhat[_qp] *= std::cbrt(ave_Fhat.det() / _Fhat[_qp].det());

    computeQpStrain();
  }
}

void
ADCompute2DFiniteStrain::displacementIntegrityCheck()
{
  if (_out_of_plane_direction != 2 && _ndisp != 3)
    mooseError("For 2D simulations where the out-of-plane direction is x or y the number of "
               "supplied displacements must be three.");
  else if (_out_of_plane_direction == 2 && _ndisp != 2)
    mooseError("For 2D simulations where the out-of-plane direction is z the number of supplied "
               "displacements must be two.");
}
