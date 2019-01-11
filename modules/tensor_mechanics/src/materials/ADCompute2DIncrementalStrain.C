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

defineADValidParams(ADCompute2DIncrementalStrain,
                    ADComputeIncrementalSmallStrain,
                    params.addClassDescription(
                        "Compute strain increment for incremental strains in 2D geometries.");

                    MooseEnum outOfPlaneDirection("x y z", "z");
                    params.addParam<MooseEnum>("out_of_plane_direction",
                                               outOfPlaneDirection,
                                               "The direction of the out-of-plane strain.");
                    return params;);

template <ComputeStage compute_stage>
ADCompute2DIncrementalStrain<compute_stage>::ADCompute2DIncrementalStrain(
    const InputParameters & parameters)
  : ADComputeIncrementalSmallStrain<compute_stage>(parameters),
    _out_of_plane_direction(adGetParam<MooseEnum>("out_of_plane_direction"))
{
}

template <ComputeStage compute_stage>
void
ADCompute2DIncrementalStrain<compute_stage>::initialSetup()
{
  for (unsigned int i = 0; i < 3; ++i)
  {
    if (_out_of_plane_direction == i)
    {
      _disp[i] = &adZeroValue();
      _grad_disp[i] = &adZeroGradient();
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

template <ComputeStage compute_stage>
void
ADCompute2DIncrementalStrain<compute_stage>::computeTotalStrainIncrement(
    ADRankTwoTensor & total_strain_increment)
{
  // Deformation gradient calculation for 2D problems
  ADRankTwoTensor A(
      (*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp], (*_grad_disp[2])[_qp]); // Deformation gradient
  RankTwoTensor Fbar((*_grad_disp_old[0])[_qp],
                     (*_grad_disp_old[1])[_qp],
                     (*_grad_disp_old[2])[_qp]); // Old Deformation gradient

  // Compute the displacement gradient of the out of plane direction for plane strain,
  // generalized plane strain, or axisymmetric problems
  A(_out_of_plane_direction, _out_of_plane_direction) = computeOutOfPlaneGradDisp();
  Fbar(_out_of_plane_direction, _out_of_plane_direction) = computeOutOfPlaneGradDispOld();

  A -= Fbar; // very nearly A = gradU - gradUold

  total_strain_increment = 0.5 * (A + A.transpose());
}

template <ComputeStage compute_stage>
void
ADCompute2DIncrementalStrain<compute_stage>::displacementIntegrityCheck()
{
  if (_out_of_plane_direction != 2 && _ndisp != 3)
    mooseError("For 2D simulations where the out-of-plane direction is x or y the number of "
               "supplied displacements must be three.");
  else if (_out_of_plane_direction == 2 && _ndisp != 2)
    mooseError("For 2D simulations where the out-of-plane direction is z the number of supplied "
               "displacements must be two.");
}

// explicit instantiation is required for AD base classes
adBaseClass(ADCompute2DIncrementalStrain);
