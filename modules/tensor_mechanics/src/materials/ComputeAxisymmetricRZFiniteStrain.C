//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeAxisymmetricRZFiniteStrain.h"
#include "Assembly.h"
#include "FEProblem.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<ComputeAxisymmetricRZFiniteStrain>()
{
  InputParameters params = validParams<Compute2DFiniteStrain>();
  params.addClassDescription(
      "Compute a strain increment for finite strains under axisymmetric assumptions.");
  return params;
}

ComputeAxisymmetricRZFiniteStrain::ComputeAxisymmetricRZFiniteStrain(
    const InputParameters & parameters)
  : Compute2DFiniteStrain(parameters), _disp_old_0(coupledValueOld("displacements", 0))
{
}

void
ComputeAxisymmetricRZFiniteStrain::initialSetup()
{
  if (getBlockCoordSystem() != Moose::COORD_RZ)
    mooseError("The coordinate system must be set to RZ for Axisymmetric geometries.");
}

Real
ComputeAxisymmetricRZFiniteStrain::computeGradDispZZ()
{
  if (!MooseUtils::absoluteFuzzyEqual(_q_point[_qp](0), 0.0))
    return (*_disp[0])[_qp] / _q_point[_qp](0);
  else
    return 0.0;
}

Real
ComputeAxisymmetricRZFiniteStrain::computeGradDispZZOld()
{
  if (!MooseUtils::absoluteFuzzyEqual(_q_point[_qp](0), 0.0))
    return _disp_old_0[_qp] / _q_point[_qp](0);
  else
    return 0.0;
}
