//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMCoreMeshGeneratorBase.h"
#include "CastUniquePointer.h"
#include "MooseApp.h"
#include "MooseMeshUtils.h"
#include "SubChannelMesh.h"

#include "libmesh/mesh_modification.h"
#include "libmesh/unstructured_mesh.h"

InputParameters
SCMCoreMeshGeneratorBase::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<std::vector<MeshGeneratorName>>(
      "inputs",
      "The SCM assembly, duct, or detailed assembly meshes that can be placed in the core");
  params.addRequiredParam<std::vector<std::vector<int>>>(
      "pattern", "Core pattern of indices into 'inputs'; -1 marks an empty position.");
  params.addRequiredRangeCheckedParam<Real>(
      "assembly_pitch",
      "assembly_pitch>0",
      "Center-to-center distance between neighboring assembly positions [m]");
  return params;
}

SCMCoreMeshGeneratorBase::SCMCoreMeshGeneratorBase(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input_names(getParam<std::vector<MeshGeneratorName>>("inputs")),
    _meshes(getMeshes("inputs")),
    _pattern(getParam<std::vector<std::vector<int>>>("pattern")),
    _assembly_pitch(getParam<Real>("assembly_pitch"))
{
  // Each SCM assembly and duct mesh generator stores its channel, gap and pin maps in the single
  // SubChannelMesh, so with several assemblies only the data of the last generated one remains,
  // and it refers to nodes of a mesh that is merged here. The SCM solver cannot use a core built
  // from those generators until that data is stored per assembly. A core of detailed assembly
  // inputs is a plain [Mesh] and can be used in a run.
  if (dynamic_cast<const SubChannelMesh *>(_mesh) && !_app.isParamSetByUser("mesh_only"))
    mooseError("An SCM core mesh built from subchannel assembly or duct inputs can only be "
               "generated with --mesh-only; solving on it is not supported yet.");
}

void
SCMCoreMeshGeneratorBase::addAssembly(const int entry, const Point & center)
{
  if (entry == -1)
    return;
  if (entry < -1 || entry >= static_cast<int>(_input_names.size()))
    paramError("pattern", "Entry ", entry, " is neither -1 nor a valid index into 'inputs'.");
  _assemblies.emplace_back(entry, center);
}

std::unique_ptr<MeshBase>
SCMCoreMeshGeneratorBase::generate()
{
  std::vector<bool> used(_input_names.size(), false);
  for (const auto & [input, _] : _assemblies)
    used[input] = true;
  for (const auto i : index_range(_input_names))
    if (!used[i])
      paramWarning("inputs", "Input '", _input_names[i], "' is not used in 'pattern'.");

  readAssemblySizes();

  // All pairs are checked, not only neighbors, because an assembly next to an empty position may
  // extend past its lattice cell into a farther occupied one. Every overlapping pair is reported at
  // once so the user does not have to fix and rerun one pair at a time.
  std::string overlaps;
  for (const auto a : index_range(_assemblies))
    for (const auto b : make_range(a + 1, _assemblies.size()))
    {
      const auto & [input_a, center_a] = _assemblies[a];
      const auto & [input_b, center_b] = _assemblies[b];
      if (overlap(input_a, input_b, center_b - center_a))
        overlaps += "\n  Assembly " + std::to_string(a) + " ('" + _input_names[input_a] +
                    "') and assembly " + std::to_string(b) + " ('" + _input_names[input_b] + "')";
    }
  if (!overlaps.empty())
    paramError("assembly_pitch",
               "The following assemblies overlap. Increase 'assembly_pitch' or use smaller "
               "assemblies.",
               overlaps);

  std::vector<std::unique_ptr<MeshBase>> inputs;
  for (const auto i : index_range(_meshes))
  {
    inputs.push_back(std::move(*_meshes[i]));
    if (!inputs.back()->is_replicated())
      paramError("inputs",
                 "Input '",
                 _input_names[i],
                 "' is not a replicated mesh. SCM core meshes require a replicated mesh; run "
                 "without --distributed-mesh.");
  }

  auto mesh = dynamic_pointer_cast<UnstructuredMesh>(buildMeshBaseObject());
  mesh->set_spatial_dimension(3);

  // Every subdomain and boundary of every placed assembly gets a new id and its name gets the
  // assembly index as a suffix, so each assembly can be addressed by block or boundary
  subdomain_id_type next_subdomain_id = 0;
  boundary_id_type next_boundary_id = 0;
  for (const auto k : index_range(_assemblies))
  {
    const auto & [input, center] = _assemblies[k];
    auto assembly = dynamic_pointer_cast<UnstructuredMesh>(inputs[input]->clone());
    const std::string suffix = "_" + std::to_string(k);

    std::set<subdomain_id_type> subdomain_ids;
    for (const auto * const elem : assembly->element_ptr_range())
      subdomain_ids.insert(elem->subdomain_id());
    std::map<subdomain_id_type, subdomain_id_type> subdomain_map;
    const auto & old_subdomain_names = assembly->get_subdomain_name_map();
    std::map<subdomain_id_type, std::string> subdomain_names;
    for (const auto old_id : subdomain_ids)
    {
      subdomain_map[old_id] = next_subdomain_id;
      const auto it = old_subdomain_names.find(old_id);
      subdomain_names[next_subdomain_id] =
          (it == old_subdomain_names.end() ? std::to_string(old_id) : it->second) + suffix;
      ++next_subdomain_id;
    }
    assembly->set_subdomain_name_map() = subdomain_names;

    if (assembly->has_elem_integer("assembly_id"))
      paramError("inputs",
                 "Input '",
                 _input_names[input],
                 "' already has an 'assembly_id' extra element integer, which this mesh "
                 "generator would overwrite.");
    const auto assembly_id_index = assembly->add_elem_integer("assembly_id");
    for (auto * const elem : assembly->element_ptr_range())
    {
      elem->subdomain_id() = libmesh_map_find(subdomain_map, elem->subdomain_id());
      elem->set_extra_integer(assembly_id_index, k);
    }

    auto & boundary_info = assembly->get_boundary_info();
    const auto side_list = boundary_info.build_side_list();
    const auto node_list = boundary_info.build_node_list();
    const auto sideset_names = boundary_info.get_sideset_name_map();
    const auto nodeset_names = boundary_info.get_nodeset_name_map();
    std::map<boundary_id_type, boundary_id_type> boundary_map;
    const auto map_boundary = [&boundary_map, &next_boundary_id](const boundary_id_type old_id)
    {
      const auto [it, inserted] = boundary_map.try_emplace(old_id, next_boundary_id);
      if (inserted)
        ++next_boundary_id;
      return it->second;
    };
    for (const auto old_id : boundary_info.get_boundary_ids())
      map_boundary(old_id);
    for (const auto & [old_id, _] : sideset_names)
      map_boundary(old_id);
    for (const auto & [old_id, _] : nodeset_names)
      map_boundary(old_id);

    // SCM assembly, duct, and detailed assembly meshes carry only side and node boundaries, so the
    // edge and shellface boundaries that clear() also drops are not restored
    boundary_info.clear();
    for (const auto & [elem_id, side, old_id] : side_list)
      boundary_info.add_side(elem_id, side, libmesh_map_find(boundary_map, old_id));
    for (const auto & [node_id, old_id] : node_list)
      boundary_info.add_node(node_id, libmesh_map_find(boundary_map, old_id));
    for (const auto & [old_id, old_name] : sideset_names)
      boundary_info.sideset_name(libmesh_map_find(boundary_map, old_id)) = old_name + suffix;
    for (const auto & [old_id, old_name] : nodeset_names)
      boundary_info.nodeset_name(libmesh_map_find(boundary_map, old_id)) = old_name + suffix;

    MeshTools::Modification::translate(*assembly, center(0), center(1), 0.0);
    MooseMeshUtils::copyIntoMesh(*this,
                                 *mesh,
                                 *assembly,
                                 /*avoid_merging_subdomains=*/false,
                                 /*avoid_merging_boundaries=*/false,
                                 _communicator);
  }

  mesh->unset_is_prepared();
  return dynamic_pointer_cast<MeshBase>(mesh);
}
