//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElemSideNeighborLayersGeomTester.h"
#include "MooseMesh.h"

// invalid_processor_id
#include "libmesh/dof_object.h"

registerMooseObject("MooseTestApp", ElemSideNeighborLayersGeomTester);

template <>
InputParameters
validParams<ElemSideNeighborLayersGeomTester>()
{
  InputParameters params = validParams<ElemSideNeighborLayersTester>();

  /**
   * Reuse an existing RelationshipManager, but restrict it to only acting geometrically.
   * There is no new code or options in this class, just a registration change.
   */
  params.registerRelationshipManagers("ElementSideNeighborLayers", "GEOMETRIC");
  params.addRequiredParam<unsigned short>("element_side_neighbor_layers",
                                          "Number of layers to ghost");

  params.addClassDescription("User object to calculate ghosted elements on a single processor or "
                             "the union across all processors.");
  return params;
}

ElemSideNeighborLayersGeomTester::ElemSideNeighborLayersGeomTester(
    const InputParameters & parameters)
  : ElemSideNeighborLayersTester(parameters)
{
}
