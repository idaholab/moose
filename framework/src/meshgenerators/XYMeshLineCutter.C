//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "XYMeshLineCutter.h"
#include "MooseMeshXYCuttingUtils.h"
#include "MooseMeshUtils.h"

#include "libmesh/mesh_modification.h"

// C++ includes
#include <cmath>

registerMooseObject("MooseApp", XYMeshLineCutter);

InputParameters
XYMeshLineCutter::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  MooseEnum cutting_type("CUT_ELEM_TRI MOV_NODE", "CUT_ELEM_TRI");
  params.addParam<MooseEnum>(
      "cutting_type",
      cutting_type,
      "Which method is to be used to cut the input mesh. 'CUT_ELEM_TRI' is the recommended method "
      "but it may cause fine elements near the cutting line, while 'MOV_NODE' deforms subdomain "
      "boundaries if they are not perpendicular to the cutting line.");

  params.addRequiredParam<MeshGeneratorName>("input", "The input mesh that needs to be trimmed.");
  params.addRequiredParam<std::vector<Real>>(
      "cut_line_params",
      "Cutting line parameters, which are a, b, and c in line equation a*x+b*y+c=0. Note that "
      "a*x+b*y+c>0 part is being removed.");
  params.addRequiredParam<boundary_id_type>(
      "new_boundary_id", "Boundary id to be assigned to the boundary formed by the cutting.");
  params.addParam<boundary_id_type>("input_mesh_external_boundary_id",
                                    "Boundary id of the external boundary of the input mesh.");
  params.addParam<std::vector<boundary_id_type>>(
      "other_boundaries_to_conform",
      "IDs of the other boundaries that need to be conformed to during nodes moving.");

  params.addParam<SubdomainName>(
      "tri_elem_subdomain_name_suffix",
      "trimmer_tri",
      "Suffix to the block name used for quad elements that are trimmed/converted into "
      "triangular elements to avert degenerate quad elements");
  params.addParam<subdomain_id_type>(
      "tri_elem_subdomain_shift",
      "Customized id shift to define subdomain ids of the converted triangular elements.");
  params.addParam<bool>(
      "improve_tri_elements", false, "Whether to improve TRI3 elements after CUT_ELEM_TRI method.");

  params.addClassDescription(
      "This XYMeshLineCutter object is designed to trim the input mesh by removing all the "
      "elements on one side of a given straight line with special processing on the elements "
      "crossed by the cutting line to ensure a smooth cross-section.");

  return params;
}

XYMeshLineCutter::XYMeshLineCutter(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _cutting_type(getParam<MooseEnum>("cutting_type").template getEnum<CutType>()),
    _input_name(getParam<MeshGeneratorName>("input")),
    _cut_line_params(getParam<std::vector<Real>>("cut_line_params")),
    _new_boundary_id(getParam<boundary_id_type>("new_boundary_id")),
    _input_mesh_external_boundary_id(
        isParamValid("input_mesh_external_boundary_id")
            ? getParam<boundary_id_type>("input_mesh_external_boundary_id")
            : Moose::INVALID_BOUNDARY_ID),
    _other_boundaries_to_conform(
        isParamValid("other_boundaries_to_conform")
            ? getParam<std::vector<boundary_id_type>>("other_boundaries_to_conform")
            : std::vector<boundary_id_type>()),
    _tri_elem_subdomain_name_suffix(getParam<SubdomainName>("tri_elem_subdomain_name_suffix")),
    _tri_elem_subdomain_shift(isParamValid("tri_elem_subdomain_shift")
                                  ? getParam<subdomain_id_type>("tri_elem_subdomain_shift")
                                  : Moose::INVALID_BLOCK_ID),
    _improve_tri_elements(getParam<bool>("improve_tri_elements")),
    _input(getMeshByName(_input_name))
{
  if (_cut_line_params.size() != 3)
    paramError("cut_line_params", "this parameter must have three elements.");
  if (MooseUtils::absoluteFuzzyEqual(_cut_line_params[0], 0.0) &&
      MooseUtils::absoluteFuzzyEqual(_cut_line_params[1], 0.0))
    paramError("cut_line_params", "At lease one of the first two elements must be non-zero.");
  if (_cutting_type == CutType::MOV_NODE && _improve_tri_elements)
    paramError("improve_tri_elements",
               "This parameter is not supported when 'MOV_NODE' method is selected as "
               "'cutting_type'.");
  if (_input_mesh_external_boundary_id == Moose::INVALID_BOUNDARY_ID &&
      _cutting_type == CutType::MOV_NODE)
    paramError(
        "input_mesh_external_boundary_id",
        "This parameter must be provided if 'MOV_NODE' method is selected as 'cutting_type'.");
}

std::unique_ptr<MeshBase>
XYMeshLineCutter::generate()
{
  auto replicated_mesh_ptr = dynamic_cast<ReplicatedMesh *>(_input.get());
  if (!replicated_mesh_ptr)
    paramError("input", "Input is not a replicated mesh, which is required");
  if (*(replicated_mesh_ptr->elem_dimensions().begin()) != 2 ||
      *(replicated_mesh_ptr->elem_dimensions().rbegin()) != 2)
    paramError("input", "Only 2D meshes are supported.");

  // Check that the input boundaries are part of the mesh
  if (isParamValid("input_mesh_external_boundary_id"))
    if (!MooseMeshUtils::hasBoundaryID(*replicated_mesh_ptr, _input_mesh_external_boundary_id))
      paramError("input_mesh_external_boundary_id", "Boundary must exist in input mesh");

  // Check that the other boundaries to conform to are part of the mesh
  if (isParamValid("other_boundaries_to_conform"))
    for (const auto bid : _other_boundaries_to_conform)
      if (!MooseMeshUtils::hasBoundaryID(*replicated_mesh_ptr, bid))
        paramError("other_boundaries_to_conform",
                   "Boundary '" + std::to_string(bid) + "' must exist in input mesh");

  ReplicatedMesh & mesh = *replicated_mesh_ptr;
  // Subdomain ID for TRI elements arising of QUAD element cuts must be new
  std::set<subdomain_id_type> subdomain_ids_set;
  mesh.subdomain_ids(subdomain_ids_set);
  const subdomain_id_type max_subdomain_id = *subdomain_ids_set.rbegin();
  const subdomain_id_type block_id_to_remove = max_subdomain_id + 1;
  const subdomain_id_type tri_subdomain_id_shift =
      _tri_elem_subdomain_shift == Moose::INVALID_BLOCK_ID ? max_subdomain_id + 2
                                                           : _tri_elem_subdomain_shift;

  // Use a unique new boundary id as the temporary boundary id for _new_boundary_id
  // This help prevent issues when _new_boundary_id is already used in the mesh
  boundary_id_type new_boundary_id_tmp = MooseMeshUtils::getNextFreeBoundaryID(mesh);

  if (_cutting_type == CutType::CUT_ELEM_TRI)
  {
    try
    {
      MooseMeshXYCuttingUtils::lineRemoverCutElem(mesh,
                                                  _cut_line_params,
                                                  tri_subdomain_id_shift,
                                                  _tri_elem_subdomain_name_suffix,
                                                  block_id_to_remove,
                                                  new_boundary_id_tmp,
                                                  _improve_tri_elements);
    }
    catch (MooseException & e)
    {
      if (((std::string)e.what()).compare("The new subdomain name already exists in the mesh.") ==
          0)
        paramError("tri_elem_subdomain_name_suffix", e.what());
      else
        mooseError("In XYMeshLineCutter with 'CUT_ELEM_TRI' mode, " + (std::string)e.what());
    }
  }
  else
  {
    try
    {
      MooseMeshXYCuttingUtils::lineRemoverMoveNode(mesh,
                                                   _cut_line_params,
                                                   block_id_to_remove,
                                                   subdomain_ids_set,
                                                   new_boundary_id_tmp,
                                                   _input_mesh_external_boundary_id,
                                                   _other_boundaries_to_conform);
    }
    catch (MooseException & e)
    {
      if (((std::string)e.what())
              .compare("The input mesh has degenerate quad element before trimming.") == 0)
        paramError("input", e.what());
      else if (((std::string)e.what())
                   .compare("The new subdomain name already exists in the mesh.") == 0)
        paramError("tri_elem_subdomain_name_suffix", e.what());
      else
        mooseError("In XYMeshLineCutter with 'MOV_NODE' mode, " + (std::string)e.what());
    }
    MooseMeshXYCuttingUtils::quasiTriElementsFixer(
        mesh, subdomain_ids_set, tri_subdomain_id_shift, _tri_elem_subdomain_name_suffix);
  }

  // Then rename the temporary boundary id to _new_boundary_id
  MeshTools::Modification::change_boundary_id(mesh, new_boundary_id_tmp, _new_boundary_id);

  mesh.prepare_for_use();
  return std::move(_input);
}
