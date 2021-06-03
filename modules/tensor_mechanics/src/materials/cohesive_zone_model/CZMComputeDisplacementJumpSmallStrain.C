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
  if (_ndisp > 3 || _ndisp < 1)
    mooseError("the CZM material requires 1, 2 or 3 displacement variables");
}

void
CZMComputeDisplacementJumpSmallStrain::initQpStatefulProperties()
{
  // requried to promote _interface_displacement_jump to stateful in case someone needs it
  _interface_displacement_jump[_qp] = 0;
}

void
CZMComputeDisplacementJumpSmallStrain::computeLocalDisplacementJump()
{
  _interface_displacement_jump[_qp] = _Q0[_qp].transpose() * _displacement_jump_global[_qp];
}
