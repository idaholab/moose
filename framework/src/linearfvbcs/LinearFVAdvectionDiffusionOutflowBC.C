//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVAdvectionDiffusionOutflowBC.h"

registerMooseObject("MooseApp", LinearFVAdvectionDiffusionOutflowBC);

InputParameters
LinearFVAdvectionDiffusionOutflowBC::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionExtrapolatedBC::validParams();

  params.addClassDescription("Adds a boundary condition which represents a surface with outflowing "
                             "material with a constant velocity. This kernel is only compatible "
                             "with advection-diffusion problems.");
  params.addParam<bool>(
      "assume_fully_developed_flow",
      false,
      "Flag to assume a zero normal gradient (fully developed flow) at the boundary.");
  return params;
}

LinearFVAdvectionDiffusionOutflowBC::LinearFVAdvectionDiffusionOutflowBC(
    const InputParameters & parameters)
  : LinearFVAdvectionDiffusionExtrapolatedBC(parameters),
    _assume_fully_developed_flow(getParam<bool>("assume_fully_developed_flow"))
{
}

Real
LinearFVAdvectionDiffusionOutflowBC::computeBoundaryNormalGradient() const
{
  if (_assume_fully_developed_flow)
    return 0;

  const auto * elem_info = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM)
                               ? _current_face_info->elemInfo()
                               : _current_face_info->neighborInfo();

  const Real sign = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM) ? 1.0 : -1.0;

  return _var.gradSln(*elem_info) * (sign * _current_face_info->normal());
}

Real
LinearFVAdvectionDiffusionOutflowBC::computeBoundaryGradientMatrixContribution() const
{
  return 0;
}

Real
LinearFVAdvectionDiffusionOutflowBC::computeBoundaryGradientRHSContribution() const
{
  return computeBoundaryNormalGradient();
}
