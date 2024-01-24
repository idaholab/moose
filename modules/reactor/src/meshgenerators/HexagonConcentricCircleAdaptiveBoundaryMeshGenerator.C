//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HexagonConcentricCircleAdaptiveBoundaryMeshGenerator.h"

#include <cmath>

registerMooseObject("ReactorApp", HexagonConcentricCircleAdaptiveBoundaryMeshGenerator);

InputParameters
HexagonConcentricCircleAdaptiveBoundaryMeshGenerator::validParams()
{
  InputParameters params = PolygonConcentricCircleMeshGeneratorBase::validParams();
  params.addRequiredRangeCheckedParam<Real>(
      "hexagon_size", "hexagon_size>0.0", "Size of the hexagon to be generated.");
  MooseEnum hexagon_size_style("apothem radius", "apothem");
  params.addParam<MooseEnum>(
      "hexagon_size_style",
      hexagon_size_style,
      "Style in which the hexagon size is given (default: apothem i.e. half-pitch).");
  params.addParam<std::vector<unsigned int>>(
      "sides_to_adapt",
      "List of the hexagon reference side indices that correspond to the sides that need adaptive "
      "meshing. The meshes to adapt these sides to are provided in 'inputs'.");
  params.addDeprecatedParam<std::vector<MeshGeneratorName>>(
      "inputs",
      "The name list of the input meshes to adapt to.",
      "Deprecated parameter, please use 'meshes_to_adapt_to' instead.");
  params.addParam<std::vector<MeshGeneratorName>>("meshes_to_adapt_to",
                                                  "The name list of the input meshes to adapt to.");
  params.addParam<bool>("is_control_drum",
                        false,
                        "Whether this mesh is for a control drum. The value can be set as 'false' "
                        "if the user wants to use this object for other components.");
  params.setParameters<bool>("uniform_mesh_on_sides", false);
  params.setParameters<bool>("quad_center_elements", false);
  params.setParameters<unsigned int>("smoothing_max_it", 0);
  params.suppressParameter<bool>("uniform_mesh_on_sides");
  params.suppressParameter<bool>("quad_center_elements");
  params.suppressParameter<unsigned int>("smoothing_max_it");
  params.suppressParameter<Real>("center_quad_factor");
  params.addParamNamesToGroup("is_control_drum", "Control Drum");
  params.addClassDescription(
      "This HexagonConcentricCircleAdaptiveBoundaryMeshGenerator object is designed to generate "
      "hexagonal meshes with adaptive boundary to facilitate stitching.");

  return params;
}

HexagonConcentricCircleAdaptiveBoundaryMeshGenerator::
    HexagonConcentricCircleAdaptiveBoundaryMeshGenerator(const InputParameters & parameters)
  : PolygonConcentricCircleMeshGeneratorBase(parameters),
    _input_names(isParamValid("meshes_to_adapt_to")
                     ? getParam<std::vector<MeshGeneratorName>>("meshes_to_adapt_to")
                     : (isParamValid("inputs") ? getParam<std::vector<MeshGeneratorName>>("inputs")
                                               : std::vector<MeshGeneratorName>()))
{
  if (_sides_to_adapt.size() != _input_names.size())
    paramError("sides_to_adapt",
               "This parameter and meshes_to_adapt_to must have the same length.");
  if (isParamValid("inputs") && isParamValid("meshes_to_adapt_to"))
    paramError("inputs",
               "this parameter is deprecated; it cannot be provided along with the new parameter "
               "'meshes_to_adapt_to'.");
  if (isParamValid("inputs"))
    _input_ptrs = getMeshes("inputs");
  else if (isParamValid("meshes_to_adapt_to"))
    _input_ptrs = getMeshes("meshes_to_adapt_to");
  _is_control_drum_meta = getParam<bool>("is_control_drum");
  _is_general_polygon = false;
}
