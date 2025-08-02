//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StitchMeshGeneratorBase.h"

#include "MooseUtils.h"
#include "MooseMeshUtils.h"

#include "libmesh/unstructured_mesh.h"

InputParameters
StitchMeshGeneratorBase::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  MooseEnum algorithm("BINARY EXHAUSTIVE", "BINARY");

  params.addParam<Real>(
      "stitching_hmin_tolerance_factor",
      TOLERANCE,
      "Factor multiplied by the elements hmin to form a tolerance to use when stitching nodes");
  params.addParam<bool>(
      "clear_stitched_boundary_ids", true, "Whether or not to clear the stitched boundary IDs");
  params.addRequiredParam<std::vector<std::vector<std::string>>>(
      "stitch_boundaries_pairs",
      "Pairs of boundaries to be stitched together between the 1st mesh in inputs and each "
      "consecutive mesh");
  params.addParam<MooseEnum>(
      "algorithm",
      algorithm,
      "Control the use of binary search for the nodes of the stitched surfaces.");
  params.addParam<bool>(
      "verbose_stitching", false, "Whether mesh stitching should have verbose output.");

  return params;
}

StitchMeshGeneratorBase::StitchMeshGeneratorBase(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _clear_stitched_boundary_ids(getParam<bool>("clear_stitched_boundary_ids")),
    _stitch_boundaries_pairs(
        getParam<std::vector<std::vector<std::string>>>("stitch_boundaries_pairs")),
    _algorithm(parameters.get<MooseEnum>("algorithm"))
{
}

boundary_id_type
StitchMeshGeneratorBase::getBoundaryIdToStitch(const MeshBase & mesh,
                                               const std::string & input_mg_name,
                                               const BoundaryName & bname) const
{
  boundary_id_type bid;
  try
  {
    bid = MooseUtils::convert<boundary_id_type>(bname, true);
  }
  catch (...)
  {
    bid = mesh.get_boundary_info().get_id_by_name(bname);

    if (bid == BoundaryInfo::invalid_id)
      errorMissingBoundary(mesh, input_mg_name, bname);
  }
  return bid;
}

void
StitchMeshGeneratorBase::errorMissingBoundary(const MeshBase & mesh,
                                              const std::string & input_mg_name,
                                              const BoundaryName & bname) const
{
  std::stringstream error;

  error << "Boundary " << bname << " doesn't exist on mesh '" << input_mg_name << "'\n";
  error << "Boundary (sideset) names that do exist: \n";
  error << " ID : Name\n";

  auto & sideset_id_name_map = mesh.get_boundary_info().get_sideset_name_map();

  for (auto & ss_name_map_pair : sideset_id_name_map)
    error << " " << ss_name_map_pair.first << " : " << ss_name_map_pair.second << "\n";

  error << "\nBoundary (nodeset) names that do exist: \n";
  error << " ID : Name\n";

  auto & nodeset_id_name_map = mesh.get_boundary_info().get_nodeset_name_map();

  for (auto & ns_name_map_pair : nodeset_id_name_map)
    error << " " << ns_name_map_pair.first << " : " << ns_name_map_pair.second << "\n";

  paramError("stitch_boundaries_pairs", error.str());
}
