//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeAxisymmetricRZSmallStrain.h"
#include "FEProblem.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<ComputeAxisymmetricRZSmallStrain>()
{
  InputParameters params = validParams<Compute2DSmallStrain>();
  params.addClassDescription("Compute a small strain in an Axisymmetric geometry");
  return params;
}

ComputeAxisymmetricRZSmallStrain::ComputeAxisymmetricRZSmallStrain(
    const InputParameters & parameters)
  : Compute2DSmallStrain(parameters)
{
}

void
ComputeAxisymmetricRZSmallStrain::initialSetup()
{
  if (getBlockCoordSystem() != Moose::COORD_RZ)
    mooseError("The coordinate system must be set to RZ for Axisymmetric geometries.");
}

Real
ComputeAxisymmetricRZSmallStrain::computeStrainZZ()
{
  if (!MooseUtils::absoluteFuzzyEqual(_q_point[_qp](0), 0.0))
    return (*_disp[0])[_qp] / _q_point[_qp](0);
  else
    return 0.0;
}
