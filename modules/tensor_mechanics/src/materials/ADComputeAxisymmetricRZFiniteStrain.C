//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeAxisymmetricRZFiniteStrain.h"
#include "Assembly.h"
#include "FEProblem.h"
#include "MooseMesh.h"

registerMooseObject("TensorMechanicsApp", ADComputeAxisymmetricRZFiniteStrain);

InputParameters
ADComputeAxisymmetricRZFiniteStrain::validParams()
{
  InputParameters params = ADCompute2DFiniteStrain::validParams();
  params.addClassDescription(
      "Compute a strain increment for finite strains under axisymmetric assumptions.");
  return params;
}

ADComputeAxisymmetricRZFiniteStrain::ADComputeAxisymmetricRZFiniteStrain(
    const InputParameters & parameters)
  : ADCompute2DFiniteStrain(parameters), _disp_old_0(coupledValueOld("displacements", 0))
{
}

void
ADComputeAxisymmetricRZFiniteStrain::initialSetup()
{
  ADCompute2DFiniteStrain::initialSetup();

  if (getBlockCoordSystem() != Moose::COORD_RZ)
    mooseError("The coordinate system must be set to RZ for Axisymmetric geometries.");

  if (_out_of_plane_direction != 2)
    paramError("out_of_plane_direction",
               "The out-of-plane direction for axisymmetric systems is currently restricted to z");
}

ADReal
ADComputeAxisymmetricRZFiniteStrain::computeOutOfPlaneGradDisp()
{
  if (!MooseUtils::absoluteFuzzyEqual(_q_point[_qp](0), 0.0))
    return (*_disp[0])[_qp] / _q_point[_qp](0);
  else
    return 0.0;
}

Real
ADComputeAxisymmetricRZFiniteStrain::computeOutOfPlaneGradDispOld()
{
  if (!MooseUtils::absoluteFuzzyEqual(_q_point[_qp](0), 0.0))
    return _disp_old_0[_qp] / _q_point[_qp](0);
  else
    return 0.0;
}
