//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMTriCoreMeshGenerator.h"
#include "CastUniquePointer.h"
#include "MooseApp.h"
#include "MooseMeshUtils.h"
#include "MooseUtils.h"
#include "SubChannelMesh.h"

#include "libmesh/mesh_modification.h"
#include "libmesh/unstructured_mesh.h"

#include <array>
#include <cmath>

registerMooseObject("SubChannelApp", SCMTriCoreMeshGenerator);

InputParameters
SCMTriCoreMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addClassDescription(
      "Places triangular-lattice subchannel assembly meshes on a hexagonal core lattice. Each "
      "assembly keeps its own subdomains and boundaries; assemblies are not connected.");
  params.addRequiredParam<std::vector<MeshGeneratorName>>(
      "inputs",
      "The SCMTriAssemblyMeshGenerator, SCMTriDuctMeshGenerator, or "
      "SCMDetailedTriAssemblyMeshGenerator meshes that can be placed in the core");
  params.addRequiredParam<std::vector<std::vector<int>>>(
      "pattern",
      "Hexagonal core pattern of indices into 'inputs', using the row syntax of "
      "PatternedHexMeshGenerator; -1 marks an empty position. Row i is placed as the i-th column "
      "from -x, and the entries of a row are placed from -y to +y.");
  params.addRequiredRangeCheckedParam<Real>(
      "assembly_pitch",
      "assembly_pitch>0",
      "Center-to-center distance between neighboring assembly positions [m]");
  return params;
}

SCMTriCoreMeshGenerator::SCMTriCoreMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input_names(getParam<std::vector<MeshGeneratorName>>("inputs")),
    _meshes(getMeshes("inputs")),
    _pattern(getParam<std::vector<std::vector<int>>>("pattern")),
    _assembly_pitch(getParam<Real>("assembly_pitch"))
{
  // Each SCMTriAssemblyMeshGenerator and SCMTriDuctMeshGenerator stores its channel, gap and pin
  // maps in the single TriSubChannelMesh, so with several assemblies only the data of the last
  // generated one remains, and it refers to nodes of a mesh that is merged here. The SCM solver
  // cannot use a core built from those generators until that data is stored per assembly. A core
  // of SCMDetailedTriAssemblyMeshGenerator inputs is a plain [Mesh] and can be used in a run.
  if (dynamic_cast<const SubChannelMesh *>(_mesh) && !_app.isParamSetByUser("mesh_only"))
    mooseError("An SCM core mesh built from SCMTriAssemblyMeshGenerator or "
               "SCMTriDuctMeshGenerator inputs can only be generated with --mesh-only; solving on "
               "it is not supported yet.");

  const auto n_rows = _pattern.size();
  if (n_rows % 2 == 0)
    paramError("pattern", "A hexagonal pattern must have an odd number of rows.");

  // Number of positions along each side of the hexagonal pattern
  const auto n_side = (n_rows + 1) / 2;
  for (const auto i : index_range(_pattern))
  {
    const auto row_length = n_side + std::min(i, n_rows - 1 - i);
    if (_pattern[i].size() != row_length)
      paramError("pattern",
                 "Row ",
                 i,
                 " has ",
                 _pattern[i].size(),
                 " entries; a hexagonal pattern with ",
                 n_rows,
                 " rows requires ",
                 row_length,
                 ".");

    for (const auto j : index_range(_pattern[i]))
    {
      const int entry = _pattern[i][j];
      if (entry == -1)
        continue;
      if (entry < -1 || entry >= static_cast<int>(_input_names.size()))
        paramError("pattern", "Entry ", entry, " is neither -1 nor a valid index into 'inputs'.");

      // The SCM assemblies have a duct corner on the +x axis, so their flat normals point at 30,
      // 90 and 150 degrees. Placing rows as columns, with neighboring columns offset by half a
      // pitch, puts every pair of neighboring positions along one of those normals.
      const Real x = (Real(i) - Real(n_side - 1)) * _assembly_pitch * std::sqrt(3.0) / 2.0;
      const Real y = (Real(j) - 0.5 * Real(row_length - 1)) * _assembly_pitch;
      _assemblies.emplace_back(entry, Point(x, y, 0.0));
    }
  }

  if (_assemblies.empty())
    paramError("pattern", "The pattern must contain at least one assembly.");
}

std::unique_ptr<MeshBase>
SCMTriCoreMeshGenerator::generate()
{
  std::vector<Real> flat_to_flat;
  for (const auto & input_name : _input_names)
  {
    if (!hasMeshProperty<Real>("flat_to_flat", input_name))
      paramError("inputs",
                 "Input '",
                 input_name,
                 "' does not provide 'flat_to_flat' metadata. Inputs must be "
                 "SCMTriAssemblyMeshGenerator, SCMTriDuctMeshGenerator, or "
                 "SCMDetailedTriAssemblyMeshGenerator objects.");
    flat_to_flat.push_back(getMeshProperty<Real>("flat_to_flat", input_name));
  }

  // The ducts are hexagons with the same orientation, and the set of center offsets at which two
  // of them overlap is a hexagon of that orientation whose apothem is the sum of their apothems.
  // They therefore overlap if and only if the center offset projected on every flat normal is
  // shorter than the sum of the apothems. All pairs are checked because empty positions can
  // leave room for an assembly larger than the pitch.
  const std::array<Point, 3> flat_normals = {Point(std::sqrt(3.0) / 2.0, 0.5, 0.0),
                                             Point(0.0, 1.0, 0.0),
                                             Point(-std::sqrt(3.0) / 2.0, 0.5, 0.0)};
  // Ducts that exactly touch (assembly_pitch equal to the flat-to-flat size) are allowed; the
  // tolerance, relative to the pitch, absorbs roundoff in the lattice coordinates.
  const Real tol = libMesh::TOLERANCE * _assembly_pitch;
  for (const auto a : index_range(_assemblies))
    for (const auto b : make_range(a + 1, _assemblies.size()))
    {
      const auto & [input_a, center_a] = _assemblies[a];
      const auto & [input_b, center_b] = _assemblies[b];
      const Point offset = center_b - center_a;
      const Real sum_apothems = 0.5 * (flat_to_flat[input_a] + flat_to_flat[input_b]);
      bool overlap = true;
      for (const auto & normal : flat_normals)
        if (!MooseUtils::absoluteFuzzyLessThan(std::abs(offset * normal), sum_apothems, tol))
          overlap = false;
      if (overlap)
        paramError("assembly_pitch",
                   "Assembly ",
                   a,
                   " ('",
                   _input_names[input_a],
                   "') and assembly ",
                   b,
                   " ('",
                   _input_names[input_b],
                   "') overlap. Increase 'assembly_pitch' or use smaller assemblies.");
    }

  std::vector<std::unique_ptr<MeshBase>> inputs;
  for (const auto i : index_range(_meshes))
  {
    inputs.push_back(std::move(*_meshes[i]));
    if (!inputs.back()->is_replicated())
      paramError("inputs", "Input '", _input_names[i], "' is not a replicated mesh.");
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
