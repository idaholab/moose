//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolygonConcentricCircleMeshGenerator.h"

registerMooseObject("ReactorApp", PolygonConcentricCircleMeshGenerator);

InputParameters
PolygonConcentricCircleMeshGenerator::validParams()
{
  InputParameters params = PolygonConcentricCircleMeshGeneratorBase::validParams();
  params.addRequiredRangeCheckedParam<Real>("polygon_size",
                                            "polygon_size>0.0",
                                            "Size of the polygon to be generated (given as either "
                                            "apothem or radius depending on polygon_size_style).");
  MooseEnum polygon_size_style("apothem radius", "apothem");
  params.addParam<MooseEnum>("polygon_size_style",
                             polygon_size_style,
                             "Style in which polygon size is given (default: apothem).");
  params.addRangeCheckedParam<unsigned int>(
      "num_sides", 6, "num_sides>=3", "Number of sides of the polygon.");
  params.addClassDescription("This PolygonConcentricCircleMeshGenerator object is designed to mesh "
                             "a polygon geometry with optional rings centered inside.");

  return params;
}

PolygonConcentricCircleMeshGenerator::PolygonConcentricCircleMeshGenerator(
    const InputParameters & parameters)
  : PolygonConcentricCircleMeshGeneratorBase(parameters)
{
  _is_general_polygon = true;
  if (_quad_center_elements && (_num_sectors_per_side.front() != _num_sectors_per_side.back()))
    paramError("quad_center_elements",
               "This parameter must be false if different side sector numbers are set.");
}
