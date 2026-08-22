//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "QuadMeshPipelineGenerator.h"

#include "Factory.h"
#include "MooseApp.h"

registerMooseObject("MooseApp", QuadMeshPipelineGenerator);

InputParameters
QuadMeshPipelineGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>(
      "boundary",
      "The input MeshGenerator defining the outer boundary of the region to mesh with "
      "quadrilaterals.");
  params.addParam<std::vector<MeshGeneratorName>>(
      "holes", std::vector<MeshGeneratorName>(), "The MeshGenerators that define mesh holes.");
  params.addRangeCheckedParam<Real>(
      "desired_area",
      0,
      "desired_area>=0",
      "Desired (maximum) triangle area of the triangulation the quadrilaterals are built from, or "
      "0 to size the elements from the boundary alone.");

  params.addParam<BoundaryName>("output_boundary",
                                "Boundary name to set on the new outer boundary.");
  params.addParam<std::vector<BoundaryName>>("hole_boundaries",
                                             "Boundary names to set on the holes.");
  params.addParam<SubdomainName>("output_subdomain_name",
                                 "Subdomain name to set on the new elements.");

  params.addRangeCheckedParam<Real>(
      "eta_min",
      0.3,
      "eta_min > 0 & eta_min <= 1",
      "The quality score that a pair of adjacent triangles must reach to be merged into a "
      "quadrilateral. A perfect rectangle scores 1.");
  params.addParam<bool>("all_quad",
                        true,
                        "Whether the triangles that could not be merged are eliminated so that the "
                        "mesh consists exclusively of quadrilaterals. When false, they are kept "
                        "and the mesh is quad-dominant.");

  params.addParam<std::vector<MeshGeneratorName>>(
      "parsed_curve_generators",
      std::vector<MeshGeneratorName>(),
      "The ParsedCurveGenerators whose curves the boundaries named in 'snap_boundaries' are "
      "snapped onto, one generator per entry of that parameter.");
  params.addParam<std::vector<BoundaryName>>(
      "snap_boundaries",
      std::vector<BoundaryName>(),
      "The boundaries whose nodes are snapped onto the curves of 'parsed_curve_generators', one "
      "boundary "
      "per entry of that parameter. Use the names given in 'output_boundary' and "
      "'hole_boundaries'.");

  params.addParam<unsigned int>(
      "smooth_iterations", 1, "Number of Laplace smoothing iterations applied to the final mesh.");

  params.addParamNamesToGroup("boundary holes desired_area", "Region");
  params.addParamNamesToGroup("eta_min all_quad", "Recombination");
  params.addParamNamesToGroup("parsed_curve_generators snap_boundaries", "Boundary snapping");

  params.addClassDescription(
      "Meshes a region bounded by input curves with quadrilaterals in one step, by chaining a "
      "frontal Delaunay triangulation, the triangle-to-quadrilateral conversion, the snapping of "
      "boundary nodes onto parametric curves, and a Laplace smoothing pass.");

  return params;
}

QuadMeshPipelineGenerator::QuadMeshPipelineGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters)
{
  const auto & parsed_curve_generators =
      getParam<std::vector<MeshGeneratorName>>("parsed_curve_generators");
  const auto & snap_boundaries = getParam<std::vector<BoundaryName>>("snap_boundaries");
  if (parsed_curve_generators.size() != snap_boundaries.size())
    paramError(
        "snap_boundaries",
        "Need one entry per entry of 'parsed_curve_generators', because each boundary is snapped "
        "onto the curve of the generator it is paired with.");

  // The boundary and hole meshes are consumed by the triangulation sub-generator below rather
  // than by this generator itself, and the curve generators by the snap sub-generators
  declareMeshForSub("boundary");
  declareMeshesForSub("holes");
  declareMeshesForSubByName(parsed_curve_generators);

  {
    auto params = _app.getFactory().getValidParams("XYFrontalDelaunayGenerator");

    params.set<MeshGeneratorName>("boundary") = getParam<MeshGeneratorName>("boundary");
    params.set<std::vector<MeshGeneratorName>>("holes") =
        getParam<std::vector<MeshGeneratorName>>("holes");
    params.set<Real>("desired_area") = getParam<Real>("desired_area");
    if (isParamValid("output_boundary"))
      params.set<BoundaryName>("output_boundary") = getParam<BoundaryName>("output_boundary");
    if (isParamValid("hole_boundaries"))
      params.set<std::vector<BoundaryName>>("hole_boundaries") =
          getParam<std::vector<BoundaryName>>("hole_boundaries");
    if (isParamValid("output_subdomain_name"))
      params.set<SubdomainName>("output_subdomain_name") =
          getParam<SubdomainName>("output_subdomain_name");

    // The defaults of 'metric' and 'orientation' already give the right isosceles triangles that
    // recombine into good quadrilaterals, so they are not exposed
    addMeshSubgenerator("XYFrontalDelaunayGenerator", name() + "_frontal", params);
  }

  {
    auto params = _app.getFactory().getValidParams("TriToQuadConverter");

    params.set<MeshGeneratorName>("input") = name() + "_frontal";
    params.set<Real>("eta_min") = getParam<Real>("eta_min");
    params.set<bool>("all_quad") = getParam<bool>("all_quad");

    addMeshSubgenerator("TriToQuadConverter", name() + "_to_quad", params);
  }

  MeshGeneratorName previous = name() + "_to_quad";
  for (const auto snap_i : index_range(parsed_curve_generators))
  {
    auto params = _app.getFactory().getValidParams("MoveNodesToCurveGenerator");

    params.set<MeshGeneratorName>("input") = previous;
    params.set<BoundaryName>("boundary") = snap_boundaries[snap_i];
    params.set<MeshGeneratorName>("parsed_curve_generator") = parsed_curve_generators[snap_i];

    previous = name() + "_snap_" + std::to_string(snap_i);
    addMeshSubgenerator("MoveNodesToCurveGenerator", previous, params);
  }

  {
    auto params = _app.getFactory().getValidParams("SmoothMeshGenerator");

    params.set<MeshGeneratorName>("input") = previous;
    params.set<MooseEnum>("algorithm") = "laplace";
    params.set<unsigned int>("iterations") = getParam<unsigned int>("smooth_iterations");

    // The Laplace algorithm holds boundary nodes fixed, so the snapped geometry survives the
    // smoothing
    addMeshSubgenerator("SmoothMeshGenerator", name() + "_smooth", params);
    _build_mesh = &getMeshByName(name() + "_smooth");
  }
}

std::unique_ptr<MeshBase>
QuadMeshPipelineGenerator::generate()
{
  return std::move(*_build_mesh);
}
