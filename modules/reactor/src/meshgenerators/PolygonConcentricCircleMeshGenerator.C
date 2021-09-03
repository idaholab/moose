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

defineLegacyParams(PolygonConcentricCircleMeshGenerator);

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
                             "Style in which polygon size is given (default: apothem). Option: " +
                                 polygon_size_style.getRawNames());
  params.addRangeCheckedParam<unsigned int>(
      "num_sides", 6, "num_sides>=3", "Number of sides of the polygon.");
  params.addParam<bool>("uniform_mesh_on_sides",
                        false,
                        "Whether the side elements are reorganized to have a uniform size.");
  params.addParam<bool>(
      "quad_center_elements", false, "Whether the center elements are quad or triangular.");
  params.addParam<unsigned int>("smoothing_max_it",
                                0,
                                "Number of Laplacian smoothing iterations. This number is "
                                "disregarded when duct_sizes is present.");
  params.addClassDescription("This PolygonConcentricCircleMeshGenerator object is designed to mesh "
                             "a polygon geometry with optional rings centered inside.");

  return params;
}

PolygonConcentricCircleMeshGenerator::PolygonConcentricCircleMeshGenerator(
    const InputParameters & parameters)
  : PolygonConcentricCircleMeshGeneratorBase(parameters),
    _max_radius_meta(declareMeshProperty<Real>("max_radius_meta", 0.0))
{
  if (_has_rings)
    _max_radius_meta = _ring_radii.back();
  _uniform_mesh_on_sides = getParam<bool>("uniform_mesh_on_sides");
  _quad_center_elements = getParam<bool>("quad_center_elements");
  if (_quad_center_elements && (_num_sectors_per_side.front() != _num_sectors_per_side.back()))
    paramError("quad_center_elements",
               "This parameter must be false if different side sector numbers are set.");
  _smoothing_max_it = getParam<unsigned int>("smoothing_max_it");
  _is_control_drum_meta = declareMeshProperty<bool>("is_control_drum_meta", false);
  _is_general_polygon = true;
}
