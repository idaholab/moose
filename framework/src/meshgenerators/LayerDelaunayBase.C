//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LayerDelaunayBase.h"

InputParameters
LayerDelaunayBase::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  return params;
}

LayerDelaunayBase::LayerDelaunayBase(const InputParameters & parameters) : MeshGenerator(parameters)
{
}

MeshGeneratorName
LayerDelaunayBase::create_conformal_coating_mesh(const unsigned int num_layers,
                                                 const Real thickness,
                                                 const Real layer_bias,
                                                 const bool is_outward_coating,
                                                 const bool keep_input,
                                                 const MeshGeneratorName & input_name,
                                                 const std::vector<BoundaryName> & boundary_names,
                                                 const MooseEnum & tri_elem_type,
                                                 const SubdomainID & block_id,
                                                 const SubdomainName & block_name,
                                                 const boundary_id_type innermost_boundary_id,
                                                 const boundary_id_type outermost_boundary_id,
                                                 const MeshGeneratorName & name_suffix)
{
  const MeshGeneratorName mg_name_prefix =
      name() + MeshGeneratorName(name_suffix.empty() ? "" : ("_" + name_suffix));
  MeshGeneratorName final_mg_name(input_name);
  std::vector<Real> layerthickness_values(num_layers + 1);
  layerthickness_values.front() = 0.0;
  const Real unit_layerthickness =
      layer_bias == 1 ? (thickness / num_layers)
                      : (thickness / (std::pow(layer_bias, num_layers) - 1.0) * (layer_bias - 1.0));
  for (const auto & layer_i :
       make_range(std::vector<Real>::size_type(1), layerthickness_values.size()))
    layerthickness_values[layer_i] = unit_layerthickness * std::pow(layer_bias, layer_i - 1);
  for (const auto & layer_i : make_range(layerthickness_values.size()))
  {
    const unsigned int layer_index = is_outward_coating ? layer_i : (num_layers - layer_i);
    const MeshGeneratorName submg_name = mg_name_prefix + "_gline_" + std::to_string(layer_index);
    const MeshGeneratorName submg_input_name =
        layer_i == 0
            ? input_name
            : MeshGeneratorName(mg_name_prefix + "_gline_" +
                                std::to_string(layer_index + (is_outward_coating ? -1 : 1)));
    auto params = _app.getFactory().getValidParams("GapLineMeshGenerator");
    if (layer_i == 0 && boundary_names.size())
      params.set<std::vector<BoundaryName>>("boundary_names") = boundary_names;
    params.set<MeshGeneratorName>("input") = submg_input_name;
    params.set<Real>("thickness") = layerthickness_values[layer_i];
    params.set<MooseEnum>("gap_direction") = is_outward_coating ? "OUTWARD" : "INWARD";
    params.set<bool>("skip_node_reduction") = true;
    addMeshSubgenerator("GapLineMeshGenerator", submg_name, params);
  }
  for (const auto & layer_i : make_range(num_layers))
  {
    const MeshGeneratorName submg_name = mg_name_prefix + "_xyd_" + std::to_string(layer_i);
    const MeshGeneratorName ob_submg_name =
        mg_name_prefix + "_gline_" + std::to_string(layer_i + 1);
    const MeshGeneratorName hole_submg_name =
        layer_i == 0 ? MeshGeneratorName(mg_name_prefix + "_gline_0")
                     : MeshGeneratorName(mg_name_prefix + "_xyd_" + std::to_string(layer_i - 1));
    auto params = _app.getFactory().getValidParams("XYDelaunayGenerator");
    params.set<MeshGeneratorName>("boundary") = ob_submg_name;
    params.set<std::vector<MeshGeneratorName>>("holes") = {hole_submg_name};
    params.set<bool>("refine_boundary") = false;
    params.set<bool>("verify_holes") = false;
    params.set<std::vector<bool>>("stitch_holes") = {layer_i > 0};
    params.set<std::vector<bool>>("refine_holes") = {false};
    if (tri_elem_type == "TRI6")
      params.set<MooseEnum>("tri_element_type") = "TRI6";
    else if (tri_elem_type == "TRI7")
      params.set<MooseEnum>("tri_element_type") = "TRI7";
    if (block_id != 0)
      params.set<SubdomainID>("output_subdomain_id") = block_id;
    if (block_name.size())
      params.set<SubdomainName>("output_subdomain_name") = block_name;

    addMeshSubgenerator("XYDelaunayGenerator", submg_name, params);

    final_mg_name = submg_name;
  }

  // If we need to keep the input mesh, we need to prepare the input mesh by adding a stitchable
  // boundary
  if (keep_input)
  {
    const MeshGeneratorName submg_name = mg_name_prefix + "_input_stitch_prep";
    auto params = _app.getFactory().getValidParams("SideSetsAroundSubdomainGenerator");
    params.set<MeshGeneratorName>("input") = input_name;
    params.set<std::vector<BoundaryName>>("new_boundary") = {(BoundaryName(submg_name))};
    params.set<bool>("include_only_external_sides") = true;
    addMeshSubgenerator("SideSetsAroundSubdomainGenerator", submg_name, params);
  }
  if (keep_input)
  {
    const MeshGeneratorName submg_name = mg_name_prefix + "_stitch_input";
    auto params = _app.getFactory().getValidParams("StitchMeshGenerator");
    // Here we put the coating mesh as the first input so that its boundary ids will not change
    params.set<std::vector<MeshGeneratorName>>("inputs") = {final_mg_name,
                                                            mg_name_prefix + "_input_stitch_prep"};
    params.set<std::vector<std::vector<std::string>>>("stitch_boundaries_pairs") = {
        {is_outward_coating ? std::to_string(1) : std::to_string(2 * num_layers - 2),
         BoundaryName(mg_name_prefix + "_input_stitch_prep")}};
    // Let's keep the stitched boundaries for now. We might need the innermost one of the coating in
    // some applications.
    params.set<bool>("clear_stitched_boundary_ids") = false;
    addMeshSubgenerator("StitchMeshGenerator", submg_name, params);
    final_mg_name = submg_name;
  }

  {
    // After the above subMG processing, the innermost boundary id is 1
    // The outmost boundary id is (num_layers - 1) * 2
    // Sometimes we want to keep the innermost boundary for following stitching.
    const MeshGeneratorName submg_name = mg_name_prefix + "_bdry_mods";
    auto params = _app.getFactory().getValidParams("BoundaryDeletionGenerator");
    params.set<MeshGeneratorName>("input") = final_mg_name;
    std::vector<BoundaryName> boundary_names_to_delete;
    for (const auto & boundary_id : make_range(boundary_id_type(2 * num_layers)))
    {
      if ((innermost_boundary_id != libMesh::BoundaryInfo::invalid_id && boundary_id == 1) ||
          (outermost_boundary_id != libMesh::BoundaryInfo::invalid_id &&
           boundary_id == (boundary_id_type(num_layers) - 1) * 2))
        continue;
      boundary_names_to_delete.push_back(std::to_string(boundary_id));
    }
    // Let's use this opportunity to also delete the stitch prep boundary
    if (keep_input)
      boundary_names_to_delete.push_back(BoundaryName(mg_name_prefix + "_input_stitch_prep"));
    params.set<std::vector<BoundaryName>>("boundary_names") = boundary_names_to_delete;
    addMeshSubgenerator("BoundaryDeletionGenerator", submg_name, params);

    final_mg_name = submg_name;
  }
  if (innermost_boundary_id != libMesh::BoundaryInfo::invalid_id ||
      outermost_boundary_id != libMesh::BoundaryInfo::invalid_id)
  {
    // Rename the remaining boundaries if applicable
    const MeshGeneratorName submg_name = mg_name_prefix + "_bdry_renaming";
    auto params = _app.getFactory().getValidParams("RenameBoundaryGenerator");
    params.set<MeshGeneratorName>("input") = final_mg_name;
    std::vector<BoundaryName> old_boundary_names;
    std::vector<BoundaryName> new_boundary_names;
    if (innermost_boundary_id != libMesh::BoundaryInfo::invalid_id)
    {
      old_boundary_names.push_back(std::to_string(1));
      new_boundary_names.push_back(std::to_string(innermost_boundary_id));
    }
    if (outermost_boundary_id != libMesh::BoundaryInfo::invalid_id)
    {
      old_boundary_names.push_back(std::to_string((boundary_id_type(num_layers) - 1) * 2));
      new_boundary_names.push_back(std::to_string(outermost_boundary_id));
    }
    params.set<std::vector<BoundaryName>>("old_boundary") = old_boundary_names;
    params.set<std::vector<BoundaryName>>("new_boundary") = new_boundary_names;
    addMeshSubgenerator("RenameBoundaryGenerator", submg_name, params);

    final_mg_name = submg_name;
  }
  return final_mg_name;
}
