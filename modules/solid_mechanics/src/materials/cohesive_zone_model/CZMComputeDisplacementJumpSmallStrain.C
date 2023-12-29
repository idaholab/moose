//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CZMComputeDisplacementJumpSmallStrain.h"

registerMooseObject("TensorMechanicsApp", CZMComputeDisplacementJumpSmallStrain);
registerMooseObject("TensorMechanicsApp", ADCZMComputeDisplacementJumpSmallStrain);

template <bool is_ad>
InputParameters
CZMComputeDisplacementJumpSmallStrainTempl<is_ad>::validParams()
{
  InputParameters params = CZMComputeDisplacementJumpBase<is_ad>::validParams();
  params.addClassDescription("Compute the total displacement jump across a czm interface in local "
                             "coordinates for the Small Strain kinematic formulation");

  return params;
}

template <bool is_ad>
CZMComputeDisplacementJumpSmallStrainTempl<is_ad>::CZMComputeDisplacementJumpSmallStrainTempl(
    const InputParameters & parameters)
  : CZMComputeDisplacementJumpBase<is_ad>(parameters)
{
}

template <bool is_ad>
void
CZMComputeDisplacementJumpSmallStrainTempl<is_ad>::computeLocalDisplacementJump()
{
  _interface_displacement_jump[_qp] =
      _czm_total_rotation[_qp].transpose() * _displacement_jump_global[_qp];
}

template class CZMComputeDisplacementJumpSmallStrainTempl<false>;
template class CZMComputeDisplacementJumpSmallStrainTempl<true>;
