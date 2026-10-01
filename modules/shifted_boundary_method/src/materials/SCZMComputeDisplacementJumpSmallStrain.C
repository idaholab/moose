//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCZMComputeDisplacementJumpSmallStrain.h"

registerMooseObject("ShiftedBoundaryMethodApp", SCZMComputeDisplacementJumpSmallStrain);
registerMooseObject("ShiftedBoundaryMethodApp", ADSCZMComputeDisplacementJumpSmallStrain);

template <bool is_ad>
InputParameters
SCZMComputeDisplacementJumpSmallStrainTempl<is_ad>::validParams()
{
  InputParameters params = SCZMComputeDisplacementJumpBase<is_ad>::validParams();
  params.addClassDescription("Compute the total displacement jump across a czm interface in local "
                             "coordinates for the Small Strain kinematic formulation");

  return params;
}

template <bool is_ad>
SCZMComputeDisplacementJumpSmallStrainTempl<is_ad>::SCZMComputeDisplacementJumpSmallStrainTempl(
    const InputParameters & parameters)
  : SCZMComputeDisplacementJumpBase<is_ad>(parameters)
{
}

template <bool is_ad>
void
SCZMComputeDisplacementJumpSmallStrainTempl<is_ad>::computeLocalDisplacementJump()
{
  _interface_displacement_jump[_qp] =
      _czm_total_rotation[_qp].transpose() * _displacement_jump_global[_qp];
}

template class SCZMComputeDisplacementJumpSmallStrainTempl<false>;
template class SCZMComputeDisplacementJumpSmallStrainTempl<true>;
