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

  params.addParam<bool>(
      "use_auto_area_func", false, "Use the automatic area function in the peripheral region.");
  params.addParam<Real>(
      "auto_area_func_default_size",
      0,
      "Background size for automatic area function, or 0 to use non background size");
  params.addParam<Real>("auto_area_func_default_size_dist",
                        -1.0,
                        "Effective distance of background size for automatic area "
                        "function, or negative to use non background size");
  params.addParam<unsigned int>("auto_area_function_num_points",
                                10,
                                "Maximum number of nearest points used for the inverse distance "
                                "interpolation algorithm for automatic area function calculation.");
  params.addRangeCheckedParam<Real>(
      "auto_area_function_power",
      1.0,
      "auto_area_function_power>0",
      "Polynomial power of the inverse distance interpolation algorithm for automatic area "
      "function calculation.");

  params.addParam<SubdomainName>("peripheral_ring_block_name",
                                 "The block name assigned to the created peripheral layer.");
  params.addParam<BoundaryName>(
      "external_boundary_name", "", "Optional customized external boundary name.");
  MooseEnum tri_elem_type("TRI3 TRI6 TRI7 DEFAULT", "DEFAULT");
  params.addParam<MooseEnum>(
      "tri_element_type", tri_elem_type, "Type of the triangular elements to be generated.");
  params.addClassDescription("This PeripheralTriangleMeshGenerator object is designed to generate "
                             "a triangulated mesh between a generated outer circle boundary "
                             "and a provided inner mesh.");
  params.addParamNamesToGroup("desired_area desired_area_func use_auto_area_func "
                              "auto_area_func_default_size auto_area_func_default_size_dist "
                              "auto_area_function_num_points auto_area_function_power",
                              "Peripheral Area Delaunay");
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
    _external_boundary_name(getParam<BoundaryName>("external_boundary_name"))
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
    params.set<bool>("use_auto_area_func") = getParam<bool>("use_auto_area_func");
    if (isParamSetByUser("auto_area_func_default_size"))
      params.set<Real>("auto_area_func_default_size") =
          getParam<Real>("auto_area_func_default_size");
    if (isParamSetByUser("auto_area_func_default_size_dist"))
      params.set<Real>("auto_area_func_default_size_dist") =
          getParam<Real>("auto_area_func_default_size_dist");
    if (isParamSetByUser("auto_area_function_num_points"))
      params.set<unsigned int>("auto_area_function_num_points") =
          getParam<unsigned int>("auto_area_function_num_points");
    if (isParamSetByUser("auto_area_function_power"))
      params.set<Real>("auto_area_function_power") = getParam<Real>("auto_area_function_power");
    params.set<bool>("refine_boundary") = false;
    params.set<std::vector<bool>>("refine_holes") = std::vector<bool>{false};
    params.set<std::vector<bool>>("stitch_holes") = std::vector<bool>{true};
    params.set<BoundaryName>("output_boundary") = _external_boundary_name;
    params.set<SubdomainName>("output_subdomain_name") = _peripheral_ring_block_name;
    params.set<MooseEnum>("tri_element_type") = getParam<MooseEnum>("tri_element_type");
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
