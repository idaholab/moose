//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeAxisymmetricRZIncrementalStrain.h"
#include "Assembly.h"
#include "FEProblem.h"
#include "MooseMesh.h"

registerMooseObject("TensorMechanicsApp", ComputeAxisymmetricRZIncrementalStrain);

InputParameters
ComputeAxisymmetricRZIncrementalStrain::validParams()
{
  InputParameters params = Compute2DIncrementalStrain::validParams();
  params.addClassDescription("Compute a strain increment and rotation increment for small strains "
                             "under axisymmetric assumptions.");
  return params;
}

ComputeAxisymmetricRZIncrementalStrain::ComputeAxisymmetricRZIncrementalStrain(
    const InputParameters & parameters)
  : Compute2DIncrementalStrain(parameters), _disp_old_0(coupledValueOld("displacements", 0))
{
}

void
ComputeAxisymmetricRZIncrementalStrain::initialSetup()
{
  ComputeIncrementalStrainBase::initialSetup();

  if (getBlockCoordSystem() != Moose::COORD_RZ)
    mooseError("The coordinate system must be set to RZ for Axisymmetric geometries.");

  if (_out_of_plane_direction != 2)
    paramError("out_of_plane_direction",
               "The out-of-plane direction for axisymmetric systems is currently restricted to z");
}

Real
ComputeAxisymmetricRZIncrementalStrain::computeOutOfPlaneGradDisp()
{
  if (!MooseUtils::absoluteFuzzyEqual(_q_point[_qp](0), 0.0))
    return (*_disp[0])[_qp] / _q_point[_qp](0);
  else
    return 0.0;
}

Real
ComputeAxisymmetricRZIncrementalStrain::computeOutOfPlaneGradDispOld()
{
  if (!MooseUtils::absoluteFuzzyEqual(_q_point[_qp](0), 0.0))
    return _disp_old_0[_qp] / _q_point[_qp](0);
  else
    return 0.0;
}
