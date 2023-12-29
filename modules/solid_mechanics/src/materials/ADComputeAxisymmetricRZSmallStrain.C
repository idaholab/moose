//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeAxisymmetricRZSmallStrain.h"
#include "FEProblem.h"
#include "MooseMesh.h"

registerMooseObject("TensorMechanicsApp", ADComputeAxisymmetricRZSmallStrain);

InputParameters
ADComputeAxisymmetricRZSmallStrain::validParams()
{
  InputParameters params = ADCompute2DSmallStrain::validParams();
  params.addClassDescription("Compute a small strain in an Axisymmetric geometry");
  return params;
}

ADComputeAxisymmetricRZSmallStrain::ADComputeAxisymmetricRZSmallStrain(
    const InputParameters & parameters)
  : ADCompute2DSmallStrain(parameters)
{
}

void
ADComputeAxisymmetricRZSmallStrain::initialSetup()
{
  ADCompute2DSmallStrain::initialSetup();

  if (getBlockCoordSystem() != Moose::COORD_RZ)
    mooseError("The coordinate system must be set to RZ for Axisymmetric geometries.");

  if (_out_of_plane_direction != 2)
    paramError("out_of_plane_direction",
               "The out-of-plane direction for axisymmetric systems is currently restricted to z");
}

ADReal
ADComputeAxisymmetricRZSmallStrain::computeOutOfPlaneStrain()
{
  if (!MooseUtils::absoluteFuzzyEqual(_q_point[_qp](0), 0.0))
    return (*_disp[0])[_qp] / _q_point[_qp](0);
  else
    return 0.0;
}
