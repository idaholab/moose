//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PeripheralTriangleMeshGenerator.h"

// Moose headers
#include "MooseApp.h"
#include "MooseMeshUtils.h"
#include "Factory.h"
#include "libmesh/elem.h"

registerMooseObject("ReactorApp", PeripheralTriangleMeshGenerator);

InputParameters
PeripheralTriangleMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<MeshGeneratorName>("input", "The input mesh to be modified.");
  params.addRequiredRangeCheckedParam<Real>("peripheral_ring_radius",
                                            "peripheral_ring_radius>0",
                                            "Radius of the peripheral ring to be added.");
  params.addRequiredRangeCheckedParam<unsigned int>("peripheral_ring_num_segments",
                                                    "peripheral_ring_num_segments>0",
                                                    "Number of segments of the peripheral ring.");
  params.addRangeCheckedParam<Real>(
      "desired_area",
      0,
      "desired_area>=0",
      "Desired (maximum) triangle area, or 0 to skip uniform refinement");
  params.addParam<std::string>(
      "desired_area_func",
      std::string(),
      "Desired area as a function of x,y; omit to skip non-uniform refinement");
  params.addParam<SubdomainName>("peripheral_ring_block_name",
                                 "The block name assigned to the created peripheral layer.");
  params.addParam<std::string>("external_boundary_name",
                               "Optional customized external boundary name.");
  params.addClassDescription("This PeripheralTriangleMeshGenerator object is designed to generate "
                             "a triangulated mesh between a generated outer circle boundary "
                             "and a provided inner mesh.");
  return params;
}

PeripheralTriangleMeshGenerator::PeripheralTriangleMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input_name(getParam<MeshGeneratorName>("input")),
    _peripheral_ring_radius(getParam<Real>("peripheral_ring_radius")),
    _peripheral_ring_num_segments(getParam<unsigned int>("peripheral_ring_num_segments")),
    _desired_area(getParam<Real>("desired_area")),
    _desired_area_func(getParam<std::string>("desired_area_func")),
    _peripheral_ring_block_name(isParamValid("peripheral_ring_block_name")
                                    ? getParam<SubdomainName>("peripheral_ring_block_name")
                                    : (SubdomainName) ""),
    _external_boundary_name(isParamValid("external_boundary_name")
                                ? getParam<std::string>("external_boundary_name")
                                : std::string())
{
  // Calculate outer boundary points

  std::vector<libMesh::Point> outer_polyline;
  // radial spacing
  Real d_theta = 2.0 * M_PI / _peripheral_ring_num_segments;
  for (unsigned int i = 0; i < _peripheral_ring_num_segments; i++)
  {
    // rotation angle
    Real theta = i * d_theta;
    // calculate (x, y) coords
    Real x = _peripheral_ring_radius * std::cos(theta);
    Real y = _peripheral_ring_radius * std::sin(theta);

    // add to outer boundary list
    outer_polyline.emplace_back(x, y, 0);
  }

  // Generate outer boundary polyline
  {
    auto params = _app.getFactory().getValidParams("PolyLineMeshGenerator");
    params.set<std::vector<Point>>("points") = outer_polyline;
    params.set<unsigned int>("num_edges_between_points") = 1;
    params.set<bool>("loop") = true;
    addMeshSubgenerator("PolyLineMeshGenerator", _input_name + "_periphery_polyline", params);
  }

  // Generate periphery region
  {
    declareMeshForSub("input");
    auto params = _app.getFactory().getValidParams("XYDelaunayGenerator");
    params.set<MeshGeneratorName>("boundary") =
        (MeshGeneratorName)_input_name + "_periphery_polyline";
    params.set<std::vector<MeshGeneratorName>>("holes") =
        std::vector<MeshGeneratorName>{_input_name};
    params.set<unsigned int>("add_nodes_per_boundary_segment") = 0;
    params.set<Real>("desired_area") = _desired_area;
    params.set<std::string>("desired_area_func") = _desired_area_func;
    params.set<bool>("refine_boundary") = false;
    params.set<std::vector<bool>>("refine_holes") = std::vector<bool>{false};
    params.set<std::vector<bool>>("stitch_holes") = std::vector<bool>{true};
    params.set<BoundaryName>("output_boundary") = _external_boundary_name;
    params.set<SubdomainName>("output_subdomain_name") = _peripheral_ring_block_name;
    addMeshSubgenerator("XYDelaunayGenerator", _input_name + "_periphery", params);
    _build_mesh = &getMeshByName(_input_name + "_periphery");
  }
}

std::unique_ptr<MeshBase>
PeripheralTriangleMeshGenerator::generate()
{
  (*_build_mesh)->find_neighbors();
  return std::move(*_build_mesh);
}
