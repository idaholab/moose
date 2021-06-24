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

InputParameters
CZMComputeDisplacementJumpSmallStrain::validParams()
{
  InputParameters params = CZMComputeDisplacementJumpBase::validParams();
  params.addClassDescription("Compute the total displacement jump accross a czm interface in local "
                             "coordinates for the Small Strain kinematic formulation");

  return params;
}

CZMComputeDisplacementJumpSmallStrain::CZMComputeDisplacementJumpSmallStrain(
    const InputParameters & parameters)
  : CZMComputeDisplacementJumpBase(parameters)
{
}

void
CZMComputeDisplacementJumpSmallStrain::computeLocalDisplacementJump()
{
  _interface_displacement_jump[_qp] =
      _czm_reference_rotation[_qp].transpose() * _displacement_jump_global[_qp];
}
