//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RayDistanceAux.h"

registerMooseObject("RayTracingApp", RayDistanceAux);

InputParameters
RayDistanceAux::validParams()
{
  auto params = AuxRayKernel::validParams();
  params.addClassDescription("Accumulates the distance traversed by each Ray segment into an aux "
                             "variable for the element that the segments are in.");
  return params;
}

RayDistanceAux::RayDistanceAux(const InputParameters & params) : AuxRayKernel(params) {}

void
RayDistanceAux::onSegment()
{
  addValue(_current_segment_length);
}
