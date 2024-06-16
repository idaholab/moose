//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCompute2DIncrementalStrain.h"

#include "libmesh/quadrature.h"

InputParameters
ADCompute2DIncrementalStrain::validParams()
{
  InputParameters params = ADComputeIncrementalStrain::validParams();
  params.addClassDescription("Compute strain increment for incremental strains in 2D geometries.");

  MooseEnum outOfPlaneDirection("x y z", "z");
  params.addParam<MooseEnum>(
      "out_of_plane_direction", outOfPlaneDirection, "The direction of the out-of-plane strain.");
  return params;
}

ADCompute2DIncrementalStrain::ADCompute2DIncrementalStrain(const InputParameters & parameters)
  : ADComputeIncrementalStrain(parameters),
    _out_of_plane_direction(getParam<MooseEnum>("out_of_plane_direction"))
{
}

void
ADCompute2DIncrementalStrain::initialSetup()
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
ADCompute2DIncrementalStrain::computeTotalStrainIncrement(ADRankTwoTensor & total_strain_increment)
{
  // Deformation gradient calculation for 2D problems
  auto A = ADRankTwoTensor::initializeFromRows(
      (*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp], (*_grad_disp[2])[_qp]); // Deformation gradient
  auto Fbar =
      RankTwoTensor::initializeFromRows((*_grad_disp_old[0])[_qp],
                                        (*_grad_disp_old[1])[_qp],
                                        (*_grad_disp_old[2])[_qp]); // Old Deformation gradient

  // Compute the displacement gradient of the out of plane direction for plane strain,
  // generalized plane strain, or axisymmetric problems
  A(_out_of_plane_direction, _out_of_plane_direction) = computeOutOfPlaneGradDisp();
  Fbar(_out_of_plane_direction, _out_of_plane_direction) = computeOutOfPlaneGradDispOld();

  A -= Fbar; // very nearly A = gradU - gradUold

  total_strain_increment = 0.5 * (A + A.transpose());
}

void
ADCompute2DIncrementalStrain::displacementIntegrityCheck()
{
  if (_out_of_plane_direction != 2 && _ndisp != 3)
    mooseError("For 2D simulations where the out-of-plane direction is x or y the number of "
               "supplied displacements must be three.");
  else if (_out_of_plane_direction == 2 && _ndisp != 2)
    mooseError("For 2D simulations where the out-of-plane direction is z the number of supplied "
               "displacements must be two.");
}
