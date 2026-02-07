//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementCentroidToSurfaceDistanceAux.h"
#include "ShortestDistanceToSurface.h"

// Register the new AuxKernel class name
registerMooseObject("ShiftedBoundaryMethodApp", ElementCentroidToSurfaceDistanceAux);

InputParameters
ElementCentroidToSurfaceDistanceAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addParam<UserObjectName>("distance_to_surface",
                                  "UserObject that provides distance calculations.");

  params.addClassDescription(
      "Creates a distance field based on the 'distance_to_surface' UserObject.");

  return params;
}

ElementCentroidToSurfaceDistanceAux::ElementCentroidToSurfaceDistanceAux(
    const InputParameters & parameters)
  : AuxKernel(parameters),
    _distance_to_surface(&getUserObject<ShortestDistanceToSurface>("distance_to_surface"))
{
  // Ensure this kernel is used only on elemental variables
  if (isNodal())
    paramError("variable", "This AuxKernel only supports Elemental fields");
}

// computeValue - Perform distance calculation
Real
ElementCentroidToSurfaceDistanceAux::computeValue()
{
  // distance value calculation
  const Point & pt = _current_elem->vertex_average();

  const Point & closest_vec = _distance_to_surface->distanceVector(pt);

  return closest_vec.norm();
}
