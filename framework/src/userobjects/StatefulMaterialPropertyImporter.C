//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StatefulMaterialPropertyImporter.h"
#include "FEProblemBase.h"
#include "MooseMesh.h"
#include "Assembly.h"
#include "DataIO.h"
#include "MooseUtils.h"

#include <fstream>
#include <sstream>

registerMooseObject("MooseApp", StatefulMaterialPropertyImporter);

namespace
{
std::string
prettyTypeName(const std::string & type_name)
{
  return MooseUtils::prettyCppType(libMesh::demangle(type_name.c_str()));
}

/// Human readable description of a group key for diagnostics
std::string
describe(const StatefulMaterialPropertyExporter::GroupKey & key)
{
  const auto & [subdomain_name, boundary_names, neighbor_subdomain_name] = key;
  std::string desc = "subdomain '" + subdomain_name + "'";
  if (!boundary_names.empty())
    desc += ", boundaries '" + MooseUtils::join(boundary_names, " ") + "'";
  if (!neighbor_subdomain_name.empty())
    desc += ", neighbor subdomain '" + neighbor_subdomain_name + "'";
  return desc;
}

/// Names of the storages in StatefulMaterialPropertyImporter::_storage_data for diagnostics
const std::array<std::string, 2> storage_names = {"volumetric", "boundary"};
}

InputParameters
StatefulMaterialPropertyImporter::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription(
      "Imports stateful material property data from a binary .smatprop file produced by "
      "StatefulMaterialPropertyExporter and remaps it onto the current mesh using "
      "closest-point matching within each subdomain (volumetric data) or within each group of "
      "element sides sharing subdomain, boundaries, and neighbor subdomain (boundary data). The "
      "remapped values are staged during "
      "EXEC_INITIAL so the second stateful-property initialization pass can load them through "
      "the normal restart path. Exported properties not declared as stateful in the current "
      "simulation are skipped.");
  params.addRequiredParam<std::string>(
      "file_base",
      "Base name for the .smatprop files written by StatefulMaterialPropertyExporter "
      "(e.g. 'my_sim'). The importer reads {file_base}.0.smatprop, determines the total "
      "rank count from that file's header, then reads all {file_base}.{rank}.smatprop files.");
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL};
  return params;
}

StatefulMaterialPropertyImporter::StatefulMaterialPropertyImporter(
    const InputParameters & parameters)
  : GeneralUserObject(parameters), _file_base(getParam<std::string>("file_base"))
{
  const auto & execute_on = getExecuteOnEnum();
  if (execute_on.size() != 1 || !execute_on.contains(EXEC_INITIAL))
    paramError("execute_on",
               "StatefulMaterialPropertyImporter must execute only on INITIAL so remapped data "
               "is staged before the second stateful material property initialization pass.");
}

void
StatefulMaterialPropertyImporter::initialSetup()
{
  readAllFiles();
  for (const auto i : index_range(_storage_data))
  {
    buildKDTrees(_storage_data[i]);
    buildPropertyMapping(_storage_data[i], remapStorage(i));
  }
}

MaterialPropertyStorage &
StatefulMaterialPropertyImporter::remapStorage(const unsigned int index)
{
  // Get non-const storage via the RemapKey pattern
  MaterialPropertyStorage::RemapKey key;
  return index == 0 ? _fe_problem.getMaterialPropertyStorageForRemap(key)
                    : _fe_problem.getBndMaterialPropertyStorageForRemap(key);
}

void
StatefulMaterialPropertyImporter::execute()
{
  populateRestartableMap();
}

void
StatefulMaterialPropertyImporter::readAllFiles()
{
  // Read rank-0 file first to determine the total number of export ranks.
  // Every file carries n_ranks in its header so stale files from a previous
  // run with a different rank count are automatically excluded.
  const auto rank0_file = _file_base + ".0.smatprop";
  {
    std::ifstream in(rank0_file, std::ios::binary);
    if (!in.good())
      mooseError("Failed to open '",
                 rank0_file,
                 "'. Make sure StatefulMaterialPropertyExporter has been run with "
                 "file_base = '",
                 _file_base,
                 "'.");

    unsigned int magic = 0, version = 0, n_ranks = 0;
    dataLoad(in, magic, nullptr);
    dataLoad(in, version, nullptr);
    dataLoad(in, n_ranks, nullptr);

    if (magic != StatefulMaterialPropertyExporter::file_magic)
      mooseError("Invalid .smatprop format in '", rank0_file, "'.");
    if (version != StatefulMaterialPropertyExporter::file_version)
      mooseError("Unsupported .smatprop version ",
                 version,
                 " in '",
                 rank0_file,
                 "'. Re-export the data with the current StatefulMaterialPropertyExporter.");

    mooseInfo("Reading ", n_ranks, " .smatprop file(s) from base '", _file_base, "'...");

    for (unsigned int rank = 0; rank < n_ranks; ++rank)
      readSingleFile(_file_base + "." + std::to_string(rank) + ".smatprop",
                     /*first=*/rank == 0);
  }

  std::ostringstream summary;
  for (const auto i : index_range(_storage_data))
  {
    const auto & data = _storage_data[i];
    std::size_t total_qps = 0;
    for (const auto & [_, recs] : data.stored_data)
      total_qps += recs.size();

    summary << (i ? "; " : "") << storage_names[i] << ": " << data.file_props.size()
            << " properties, " << data.stored_data.size() << " groups, " << total_qps
            << " quadrature points";
  }
  mooseInfo("Loaded ", summary.str(), ".");
}

void
StatefulMaterialPropertyImporter::readSingleFile(const std::string & filename, bool first)
{
  std::ifstream in(filename, std::ios::binary);
  if (!in.good())
    mooseError("Failed to open .smatprop file '", filename, "'.");

  // Read and validate header (magic, version, n_ranks already consumed from rank-0
  // file; re-read and discard here to stay in sync with the stream)
  unsigned int magic = 0, version = 0, n_ranks = 0;
  dataLoad(in, magic, nullptr);
  dataLoad(in, version, nullptr);
  dataLoad(in, n_ranks, nullptr);

  if (magic != StatefulMaterialPropertyExporter::file_magic)
    mooseError("Invalid .smatprop format in '", filename, "'.");
  if (version != StatefulMaterialPropertyExporter::file_version)
    mooseError("Unsupported .smatprop version in '", filename, "'.");

  for (auto & data : _storage_data)
    readStorage(in, filename, data, first);
}

void
StatefulMaterialPropertyImporter::readStorage(std::istream & in,
                                              const std::string & filename,
                                              StorageData & data,
                                              const bool first)
{
  // Property metadata
  unsigned int n_props = 0;
  dataLoad(in, n_props, nullptr);

  if (first)
  {
    // First file — read and store property registry
    data.file_props.resize(n_props);
    for (auto & fp : data.file_props)
    {
      dataLoad(in, fp.name, nullptr);
      dataLoad(in, fp.type_str, nullptr);
      dataLoad(in, fp.max_state, nullptr);
    }
  }
  else
  {
    // Subsequent files — validate registry matches (same props from the same simulation)
    if (n_props != data.file_props.size())
      mooseError("Property count mismatch between rank files: rank-0 has ",
                 data.file_props.size(),
                 " properties, '",
                 filename,
                 "' has ",
                 n_props,
                 ".");
    for (unsigned int i = 0; i < n_props; ++i)
    {
      FilePropRecord fp;
      dataLoad(in, fp.name, nullptr);
      dataLoad(in, fp.type_str, nullptr);
      dataLoad(in, fp.max_state, nullptr);
      if (fp.name != data.file_props[i].name || fp.type_str != data.file_props[i].type_str ||
          fp.max_state != data.file_props[i].max_state)
        mooseError("Property registry mismatch between rank files at index ",
                   i,
                   ": expected '",
                   data.file_props[i].name,
                   "' (",
                   prettyTypeName(data.file_props[i].type_str),
                   ", max state ",
                   data.file_props[i].max_state,
                   "), got '",
                   fp.name,
                   "' (",
                   prettyTypeName(fp.type_str),
                   ", max state ",
                   fp.max_state,
                   ") in '",
                   filename,
                   "'.");
    }
  }

  // Group data - append into stored_data (multiple ranks may share a group)
  unsigned int n_groups = 0;
  dataLoad(in, n_groups, nullptr);

  for (unsigned int g = 0; g < n_groups; ++g)
  {
    GroupKey key;
    auto & [subdomain_name, boundary_names, neighbor_subdomain_name] = key;
    dataLoad(in, subdomain_name, nullptr);
    dataLoad(in, boundary_names, nullptr);
    dataLoad(in, neighbor_subdomain_name, nullptr);

    uint64_t n_qpoints = 0;
    dataLoad(in, n_qpoints, nullptr);

    auto & records = data.stored_data[key];
    const auto offset = records.size();
    records.resize(offset + n_qpoints);

    for (uint64_t q = 0; q < n_qpoints; ++q)
    {
      auto & rec = records[offset + q];
      dataLoad(in, rec.coord, nullptr);

      rec.blobs.resize(n_props);
      for (unsigned int sid = 0; sid < n_props; ++sid)
      {
        rec.blobs[sid].resize(data.file_props[sid].max_state + 1);
        for (unsigned int state = 0; state <= data.file_props[sid].max_state; ++state)
        {
          uint64_t blob_size = 0;
          dataLoad(in, blob_size, nullptr);
          if (blob_size > 0)
          {
            rec.blobs[sid][state].resize(blob_size);
            in.read(rec.blobs[sid][state].data(), blob_size);
            if (!in.good())
              mooseError(
                  "Failed while reading property data from .smatprop file '", filename, "'.");
          }
        }
      }
    }
  }
}

void
StatefulMaterialPropertyImporter::buildKDTrees(StorageData & data)
{
  const unsigned int max_leaf_size = 20;

  for (auto & [key, records] : data.stored_data)
  {
    auto & points = data.kdtree_points[key];
    points.reserve(records.size());
    for (const auto & rec : records)
      points.push_back(rec.coord);

    if (!points.empty())
      data.kdtrees[key] = std::make_unique<KDTree>(points, max_leaf_size);
  }
}

void
StatefulMaterialPropertyImporter::buildPropertyMapping(StorageData & data,
                                                       const MaterialPropertyStorage & storage)
{
  const auto & registry = storage.getMaterialPropertyRegistry();

  // Build mapping: file stateful_id -> current stateful_id based on property name matching.
  // Properties present in the file but not declared stateful in the current simulation are
  // skipped; properties that do match must still have the same concrete type.
  data.file_to_current_sid.resize(data.file_props.size());
  for (const auto file_sid : index_range(data.file_props))
  {
    const auto & file_prop = data.file_props[file_sid];
    const auto query_id = registry.queryID(file_prop.name);
    if (!query_id)
    {
      mooseInfo("Skipping imported stateful property '",
                file_prop.name,
                "' because it does not exist in the current simulation.");
      continue;
    }

    const auto target_prop_id = *query_id;
    const auto & record = storage.getPropRecord(target_prop_id);
    if (!record.stateful())
    {
      mooseInfo("Skipping imported property '",
                file_prop.name,
                "' because it is not declared as a stateful property in the current "
                "simulation.");
      continue;
    }

    // Validate type match
    if (record.type != file_prop.type_str)
      mooseError("Type mismatch for stateful property '",
                 file_prop.name,
                 "': file has '",
                 prettyTypeName(file_prop.type_str),
                 "', current sim has '",
                 prettyTypeName(record.type),
                 "'.");
    if (record.state != file_prop.max_state)
      mooseError("State count mismatch for stateful property '",
                 file_prop.name,
                 "': file has max state ",
                 file_prop.max_state,
                 ", current sim has max state ",
                 record.state,
                 ".");
    data.file_to_current_sid[file_sid] = record.stateful_id;
  }

  // Check we have at least one match
  bool any_match = false;
  for (const auto & m : data.file_to_current_sid)
    if (m)
    {
      any_match = true;
      break;
    }

  if (!any_match && !data.file_props.empty())
    mooseInfo("No matching stateful material properties found between .smatprop file and "
              "current simulation.");
}

void
StatefulMaterialPropertyImporter::populateRestartableMap()
{
  auto & mesh = _fe_problem.mesh();
  auto & assembly = _fe_problem.assembly(0, 0);

  const auto & volume_data = _storage_data[0];
  const auto & bnd_data = _storage_data[1];
  auto & volume_storage = remapStorage(0);
  auto & bnd_storage = remapStorage(1);

  std::size_t remapped_elems = 0;
  std::size_t skipped_elems = 0;
  std::size_t remapped_sides = 0;

  for (const auto & elem : mesh.getMesh().active_local_element_ptr_range())
  {
    assembly.setCurrentSubdomainID(elem->subdomain_id());

    const auto volume_key = StatefulMaterialPropertyExporter::volumeKey(mesh, *elem);
    if (volume_data.kdtrees.count(volume_key))
    {
      // Reinit to get current qp positions
      assembly.reinit(elem);
      stageRemappedData(volume_data, volume_storage, volume_key, elem, 0, assembly.qPoints());
      remapped_elems++;
    }
    else
      skipped_elems++;

    // Stage every side whose group was exported. Sides that are not initialized with stateful
    // boundary data in this simulation leave their staged data unused.
    if (!bnd_data.kdtrees.empty())
      for (const auto side : elem->side_index_range())
      {
        const auto face_key = StatefulMaterialPropertyExporter::faceKey(mesh, *elem, side);
        if (!bnd_data.kdtrees.count(face_key))
          continue;

        assembly.reinit(elem, side);
        stageRemappedData(bnd_data, bnd_storage, face_key, elem, side, assembly.qPointsFace());
        remapped_sides++;
      }
  }

  mooseInfo("Prepared remapped stateful storage (states 0+) for ",
            remapped_elems,
            " elements (",
            skipped_elems,
            " skipped due to no subdomain match) and ",
            remapped_sides,
            " element sides.");
}

void
StatefulMaterialPropertyImporter::stageRemappedData(const StorageData & data,
                                                    MaterialPropertyStorage & storage,
                                                    const GroupKey & key,
                                                    const Elem * const elem,
                                                    const unsigned int side,
                                                    const MooseArray<Point> & q_points)
{
  auto & kdtree = *libmesh_map_find(data.kdtrees, key);
  const auto & stored_records = libmesh_map_find(data.stored_data, key);
  const auto n_qpoints = q_points.size();

  // For each current stateful_id that has a file match, stage current and historical states
  // into the restartable map. initStatefulProps() will load these values in place while
  // still running normal initialization for non-imported properties.
  for (const auto file_sid : index_range(data.file_props))
  {
    if (!data.file_to_current_sid[file_sid])
      continue;

    const auto current_sid = *data.file_to_current_sid[file_sid];
    const auto max_state = data.file_props[file_sid].max_state;

    for (unsigned int state = 0; state <= max_state; ++state)
    {
      // Assemble a blob by concatenating per-qp data from nearest stored qps.
      // This blob is compatible with PropertyValue::load() which reads n_qpoints
      // consecutive T values.
      std::stringstream assembled;

      for (unsigned int qp = 0; qp < n_qpoints; ++qp)
      {
        Point target_pt = q_points[qp];

        // Find nearest stored qp in this group
        std::vector<std::size_t> return_index;
        kdtree.neighborSearch(target_pt, 1, return_index);

        if (return_index.empty() || return_index[0] >= stored_records.size())
          mooseError("KDTree search failed for element ",
                     elem->id(),
                     " side ",
                     side,
                     " qp ",
                     qp,
                     " in ",
                     describe(key),
                     ".");

        const auto & nearest = stored_records[return_index[0]];

        if (file_sid >= nearest.blobs.size() || state >= nearest.blobs[file_sid].size() ||
            nearest.blobs[file_sid][state].empty())
          mooseError("Missing exported data for stateful property '",
                     data.file_props[file_sid].name,
                     "' state ",
                     state,
                     " in source ",
                     describe(key),
                     " while remapping element ",
                     elem->id(),
                     " side ",
                     side,
                     " qp ",
                     qp,
                     ". Ensure the exported and target simulations declare matching stateful "
                     "properties on matched subdomains and boundaries.");

        const auto & blob = nearest.blobs[file_sid][state];
        assembled.write(blob.data(), blob.size());
      }

      storage.addToRestartableMap(MaterialPropertyStorage::RemapKey(),
                                  elem,
                                  side,
                                  current_sid,
                                  state,
                                  std::move(assembled));
    }
  }
}
