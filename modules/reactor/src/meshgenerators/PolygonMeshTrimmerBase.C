//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolygonMeshTrimmerBase.h"
#include "MooseMeshXYCuttingUtils.h"
#include "MooseMeshUtils.h"
#include "MathUtils.h"

// C++ includes
#include <cmath> // provides round, not std::round (see http://www.cplusplus.com/reference/cmath/round/)

InputParameters
PolygonMeshTrimmerBase::validParams()
{
  InputParameters params = PolygonMeshGeneratorBase::validParams();
  params.addRequiredParam<MeshGeneratorName>("input", "The input mesh that needs to be trimmed.");
  params.addParam<BoundaryName>("peripheral_trimming_section_boundary",
                                "Boundary formed by peripheral trimming.");
  params.addRangeCheckedParam<short>(
      "center_trim_starting_index",
      "center_trim_starting_index>=0 & center_trim_starting_index<12",
      "Index of the starting center trimming position.");
  params.addRangeCheckedParam<short>("center_trim_ending_index",
                                     "center_trim_ending_index>=0 & center_trim_ending_index<12",
                                     "Index of the ending center trimming position.");
  params.addParam<BoundaryName>("center_trimming_section_boundary",
                                "Boundary formed by center trimming (external_boundary will be "
                                "assigned if this parameter is not provided).");
  params.addParam<BoundaryName>("external_boundary",
                                "External boundary of the input mesh prior to the trimming.");
  params.addParam<SubdomainName>(
      "tri_elem_subdomain_name_suffix",
      "trimmer_tri",
      "Suffix to the block name used for quad elements that are trimmed/converted into "
      "triangular elements to avert degenerate quad elements");
  params.addParam<subdomain_id_type>(
      "tri_elem_subdomain_shift",
      "Customized id shift to define subdomain ids of the converted triangular elements.");

  params.addParamNamesToGroup(
      "center_trim_starting_index center_trim_ending_index center_trimming_section_boundary",
      "Center Trimming");
  params.addParamNamesToGroup("peripheral_trimming_section_boundary", "Peripheral Trimming");
  params.addParamNamesToGroup("tri_elem_subdomain_name_suffix tri_elem_subdomain_shift",
                              "Trimmed Boundary Repair");

  params.addClassDescription("This PolygonMeshTrimmerBase is the base class for "
                             "CartesianMeshTrimmer and HexagonMeshTrimmer.");

  return params;
}

PolygonMeshTrimmerBase::PolygonMeshTrimmerBase(const InputParameters & parameters)
  : PolygonMeshGeneratorBase(parameters),
    _input_name(getParam<MeshGeneratorName>("input")),
    _trim_peripheral_region(getParam<std::vector<unsigned short>>("trim_peripheral_region")),
    _peripheral_trimming_section_boundary(
        isParamValid("peripheral_trimming_section_boundary")
            ? getParam<BoundaryName>("peripheral_trimming_section_boundary")
            : BoundaryName()),
    _center_trimming_section_boundary(
        isParamValid("center_trimming_section_boundary")
            ? getParam<BoundaryName>("center_trimming_section_boundary")
            : BoundaryName()),
    _external_boundary_name(isParamValid("external_boundary")
                                ? getParam<BoundaryName>("external_boundary")
                                : BoundaryName()),
    _tri_elem_subdomain_name_suffix(getParam<SubdomainName>("tri_elem_subdomain_name_suffix")),
    _tri_elem_subdomain_shift(isParamValid("tri_elem_subdomain_shift")
                                  ? getParam<subdomain_id_type>("tri_elem_subdomain_shift")
                                  : Moose::INVALID_BLOCK_ID),
    _input(getMeshByName(_input_name))
{
  declareMeshProperty("pattern_pitch_meta", 0.0);
  declareMeshProperty("input_pitch_meta", 0.0);
  declareMeshProperty("is_control_drum_meta", false);
  if (std::accumulate(_trim_peripheral_region.begin(), _trim_peripheral_region.end(), 0) == 0 &&
      !_peripheral_trimming_section_boundary.empty())
    paramError("peripheral_trimming_section_boundary",
               "this input parameter is not used if peripheral trimming is not performed.");
}

std::unique_ptr<MeshBase>
PolygonMeshTrimmerBase::generate()
{
  auto replicated_mesh_ptr = dynamic_cast<ReplicatedMesh *>(_input.get());
  if (!replicated_mesh_ptr)
    paramError("input", "Input is not a replicated mesh, which is required");

  ReplicatedMesh & mesh = *replicated_mesh_ptr;

  // Passing metadata
  if (hasMeshProperty<Real>("input_pitch_meta", _input_name))
    setMeshProperty("input_pitch_meta", getMeshProperty<Real>("input_pitch_meta", _input_name));
  if (hasMeshProperty<bool>("is_control_drum_meta", _input_name))
    setMeshProperty("is_control_drum_meta",
                    getMeshProperty<bool>("is_control_drum_meta", _input_name));

  const boundary_id_type external_boundary_id =
      _external_boundary_name.empty()
          ? (boundary_id_type)OUTER_SIDESET_ID
          : MooseMeshUtils::getBoundaryID(_external_boundary_name, mesh);
  if (external_boundary_id == libMesh::BoundaryInfo::invalid_id)
    paramError("external_boundary",
               "the provided external boundary does not exist in the input mesh.");

  std::set<subdomain_id_type> subdomain_ids_set;
  mesh.subdomain_ids(subdomain_ids_set);

  if (*max_element(_trim_peripheral_region.begin(), _trim_peripheral_region.end()))
  {
    const boundary_id_type peripheral_trimming_section_boundary_id =
        _peripheral_trimming_section_boundary.empty()
            ? external_boundary_id
            : (MooseMeshUtils::getBoundaryIDs(mesh, {_peripheral_trimming_section_boundary}, true))
                  .front();
    peripheralTrimmer(mesh,
                      _trim_peripheral_region,
                      external_boundary_id,
                      peripheral_trimming_section_boundary_id,
                      subdomain_ids_set);
    mesh.get_boundary_info().sideset_name(peripheral_trimming_section_boundary_id) =
        _peripheral_trimming_section_boundary;
  }
  else if (hasMeshProperty<Real>("pattern_pitch_meta", _input_name))
    setMeshProperty("pattern_pitch_meta", getMeshProperty<Real>("pattern_pitch_meta", _input_name));

  if (_center_trim_sector_number < _num_sides * 2)
  {
    const boundary_id_type center_trimming_section_boundary_id =
        _center_trimming_section_boundary.empty()
            ? external_boundary_id
            : (MooseMeshUtils::getBoundaryIDs(mesh, {_center_trimming_section_boundary}, true))
                  .front();
    centerTrimmer(mesh,
                  _num_sides,
                  _center_trim_sector_number,
                  _trimming_start_sector,
                  external_boundary_id,
                  center_trimming_section_boundary_id,
                  subdomain_ids_set);
    mesh.get_boundary_info().sideset_name(center_trimming_section_boundary_id) =
        _center_trimming_section_boundary;
  }

  if (MooseMeshXYCuttingUtils::quasiTriElementsFixer(
          mesh, subdomain_ids_set, _tri_elem_subdomain_shift, _tri_elem_subdomain_name_suffix))
    mesh.prepare_for_use();

  return std::move(_input);
}

void
PolygonMeshTrimmerBase::centerTrimmer(ReplicatedMesh & mesh,
                                      const unsigned int num_sides,
                                      const unsigned int center_trim_sector_number,
                                      const unsigned int trimming_start_sector,
                                      const boundary_id_type external_boundary_id,
                                      const boundary_id_type center_trimming_section_boundary_id,
                                      const std::set<subdomain_id_type> subdomain_ids_set)
{
  const subdomain_id_type max_subdomain_id = *subdomain_ids_set.rbegin();
  const subdomain_id_type block_id_to_remove = max_subdomain_id + 1;

  std::vector<std::vector<Real>> bdry_pars = {
      {std::cos((Real)trimming_start_sector * M_PI / (Real)num_sides),
       std::sin((Real)trimming_start_sector * M_PI / (Real)num_sides),
       0.0},
      {-std::cos((Real)(trimming_start_sector + center_trim_sector_number) * M_PI /
                 (Real)num_sides),
       -std::sin((Real)(trimming_start_sector + center_trim_sector_number) * M_PI /
                 (Real)num_sides),
       0.0}};

  for (unsigned int i = 0; i < bdry_pars.size(); i++)
    try
    {
      MooseMeshXYCuttingUtils::lineRemoverMoveNode(mesh,
                                                   bdry_pars[i],
                                                   block_id_to_remove,
                                                   subdomain_ids_set,
                                                   center_trimming_section_boundary_id,
                                                   external_boundary_id);
    }
    catch (MooseException & e)
    {
      if (((std::string)e.what())
              .compare("The input mesh has degenerate quad element before trimming.") == 0)
        paramError("input", "The input mesh has degenerate quad element before trimming.");
      else if (((std::string)e.what())
                   .compare("The new subdomain name already exists in the mesh.") == 0)
        paramError("tri_elem_subdomain_name_suffix",
                   "The new subdomain name already exists in the mesh.");
    }
}

void
PolygonMeshTrimmerBase::peripheralTrimmer(
    ReplicatedMesh & mesh,
    const std::vector<unsigned short> trim_peripheral_region,
    const boundary_id_type external_boundary_id,
    const boundary_id_type peripheral_trimming_section_boundary_id,
    const std::set<subdomain_id_type> subdomain_ids_set)
{
  const unsigned int num_sides = trim_peripheral_region.size();
  const subdomain_id_type max_subdomain_id = *subdomain_ids_set.rbegin();
  const subdomain_id_type block_id_to_remove = max_subdomain_id + 1;

  const Real unit_length = getMeshProperty<Real>("input_pitch_meta", _input_name) /
                           (num_sides == 6 ? std::sqrt(3.0) : 2.0);
  // Add metadata to input
  const Real multiplier = ((Real)getMeshProperty<unsigned int>("pattern_size", _input_name) - 1.0) *
                          (num_sides == 6 ? 0.75 : 1.0);
  const Real ch_length = multiplier * unit_length;
  setMeshProperty("pattern_pitch_meta", ch_length * 2.0);

  std::vector<std::vector<Real>> bdry_pars;
  if (num_sides == 6)
    bdry_pars = {{1.0 / std::sqrt(3.0), 1.0, -ch_length / std::sqrt(3.0) * 2.0},
                 {-1.0 / std::sqrt(3.0), 1.0, -ch_length / std::sqrt(3.0) * 2.0},
                 {-1.0, 0.0, -ch_length},
                 {-1.0 / std::sqrt(3.0), -1.0, -ch_length / std::sqrt(3.0) * 2.0},
                 {1.0 / std::sqrt(3.0), -1.0, -ch_length / std::sqrt(3.0) * 2.0},
                 {1.0, 0.0, -ch_length}};
  else
    bdry_pars = {{1.0, 0.0, -ch_length},
                 {0.0, 1.0, -ch_length},
                 {-1.0, 0.0, -ch_length},
                 {0.0, -1.0, -ch_length}};

  for (unsigned int i = 0; i < bdry_pars.size(); i++)
    if (trim_peripheral_region[i])
      try
      {
        MooseMeshXYCuttingUtils::lineRemoverMoveNode(mesh,
                                                     bdry_pars[i],
                                                     block_id_to_remove,
                                                     subdomain_ids_set,
                                                     peripheral_trimming_section_boundary_id,
                                                     external_boundary_id,
                                                     std::vector<boundary_id_type>(),
                                                     true);
      }
      catch (MooseException & e)
      {
        if (((std::string)e.what())
                .compare("The input mesh has degenerate quad element before trimming.") == 0)
          paramError("input", "The input mesh has degenerate quad element before trimming.");
        else if (((std::string)e.what())
                     .compare("The new subdomain name already exists in the mesh.") == 0)
          paramError("tri_elem_subdomain_name_suffix",
                     "The new subdomain name already exists in the mesh.");
      }
}
