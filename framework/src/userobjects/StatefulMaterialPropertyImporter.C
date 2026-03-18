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

registerMooseObject("MooseApp", StatefulMaterialPropertyImporter);

InputParameters
StatefulMaterialPropertyImporter::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription(
      "Imports stateful material property data from a binary .smatprop file produced by "
      "StatefulMaterialPropertyExporter and remaps it onto the current mesh using "
      "closest-point matching within each subdomain. Overwrites old/older states directly "
      "after initStatefulProperties() has run for all elements.");
  params.addRequiredParam<std::string>("file",
                                       "The .smatprop file to read stateful property data from.");
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL};
  return params;
}

StatefulMaterialPropertyImporter::StatefulMaterialPropertyImporter(
    const InputParameters & parameters)
  : GeneralUserObject(parameters), _file(getParam<std::string>("file"))
{
}

void
StatefulMaterialPropertyImporter::initialSetup()
{
  if (n_processors() > 1)
    mooseError("StatefulMaterialPropertyImporter does not yet support parallel execution. "
               "Run with a single processor or implement MPI gather in "
               "StatefulMaterialPropertyExporter first.");

  readFile();
  buildKDTrees();
  buildPropertyMapping();
}

void
StatefulMaterialPropertyImporter::execute()
{
  overwriteStorageStates();
}

void
StatefulMaterialPropertyImporter::readFile()
{
  std::ifstream in(_file, std::ios::binary);
  if (!in.good())
    mooseError("Failed to open file '", _file, "' for reading.");

  // Read and validate header
  unsigned int magic = 0, version = 0;
  dataLoad(in, magic, nullptr);
  dataLoad(in, version, nullptr);

  if (magic != 0x4D504D53)
    mooseError("Invalid file format in '", _file, "'. Expected .smatprop file.");
  if (version != 1)
    mooseError("Unsupported .smatprop file version: ", version);

  // Read property metadata
  unsigned int n_props = 0;
  dataLoad(in, n_props, nullptr);

  _file_props.resize(n_props);
  for (unsigned int i = 0; i < n_props; ++i)
  {
    dataLoad(in, _file_props[i].name, nullptr);
    dataLoad(in, _file_props[i].type_str, nullptr);
    dataLoad(in, _file_props[i].max_state, nullptr);
  }

  // Read subdomain data
  unsigned int n_subdomains = 0;
  dataLoad(in, n_subdomains, nullptr);

  for (unsigned int s = 0; s < n_subdomains; ++s)
  {
    std::string sub_name;
    dataLoad(in, sub_name, nullptr);

    uint64_t n_qpoints = 0;
    dataLoad(in, n_qpoints, nullptr);

    auto & records = _stored_data[sub_name];
    records.resize(n_qpoints);

    for (uint64_t q = 0; q < n_qpoints; ++q)
    {
      dataLoad(in, records[q].coord, nullptr);

      records[q].blobs.resize(n_props);
      for (unsigned int sid = 0; sid < n_props; ++sid)
      {
        records[q].blobs[sid].resize(_file_props[sid].max_state + 1);
        for (unsigned int state = 0; state <= _file_props[sid].max_state; ++state)
        {
          uint64_t blob_size = 0;
          dataLoad(in, blob_size, nullptr);
          if (blob_size > 0)
          {
            records[q].blobs[sid][state].resize(blob_size);
            in.read(records[q].blobs[sid][state].data(), blob_size);
          }
        }
      }
    }
  }

  std::size_t total_qps = 0;
  for (const auto & [name, recs] : _stored_data)
    total_qps += recs.size();

  mooseInfo("Loaded .smatprop: ",
            _file_props.size(),
            " properties, ",
            _stored_data.size(),
            " subdomains, ",
            total_qps,
            " quadrature points.");
}

void
StatefulMaterialPropertyImporter::buildKDTrees()
{
  const unsigned int max_leaf_size = 20;

  for (auto & [sub_name, records] : _stored_data)
  {
    auto & points = _kdtree_points[sub_name];
    points.reserve(records.size());
    for (const auto & rec : records)
      points.push_back(rec.coord);

    if (!points.empty())
      _kdtrees[sub_name] = std::make_unique<KDTree>(points, max_leaf_size);
  }
}

void
StatefulMaterialPropertyImporter::buildPropertyMapping()
{
  // Get non-const storage via the RemapKey pattern
  MaterialPropertyStorage::RemapKey key;
  auto & storage = _fe_problem.getMaterialPropertyStorageForRemap(key);

  const auto & registry = storage.getMaterialPropertyRegistry();

  // Build mapping: file stateful_id → current stateful_id
  // based on property name matching
  _file_to_current_sid.resize(_file_props.size());
  for (const auto file_sid : index_range(_file_props))
  {
    const auto & file_prop = _file_props[file_sid];
    const auto query_id = registry.queryID(file_prop.name);
    if (!query_id)
      mooseError("Stateful material property '",
                 file_prop.name,
                 "' exists in the import file but is not declared as a stateful property "
                 "in the current simulation. All properties present in the import file "
                 "must exist in the current simulation.");

    const auto target_prop_id = *query_id;
    const auto & record = storage.getPropRecord(target_prop_id);
    if (!record.stateful())
      mooseError("Stateful material property '",
                 file_prop.name,
                 "' exists in the import file but is not declared as a stateful property "
                 "in the current simulation. All properties present in the import file "
                 "must exist in the current simulation.");

    // Validate type match
    if (record.type != file_prop.type_str)
      mooseError("Type mismatch for stateful property '",
                 file_prop.name,
                 "': file has '",
                 file_prop.type_str,
                 "', current sim has '",
                 record.type,
                 "'.");
    _file_to_current_sid[file_sid] = record.stateful_id;
  }

  // Check we have at least one match
  bool any_match = false;
  for (const auto & m : _file_to_current_sid)
    if (m)
    {
      any_match = true;
      break;
    }

  if (!any_match)
    mooseWarning("No matching stateful material properties found between .smatprop file and "
                 "current simulation.");
}

void
StatefulMaterialPropertyImporter::overwriteStorageStates()
{
  // Get non-const storage via the RemapKey pattern
  MaterialPropertyStorage::RemapKey key;
  auto & storage = _fe_problem.getMaterialPropertyStorageForRemap(key);

  auto & mesh = _fe_problem.mesh();
  auto & assembly = _fe_problem.assembly(0, 0);
  const unsigned int side = 0;

  std::size_t remapped_elems = 0;
  std::size_t skipped_elems = 0;

  for (const auto & elem : mesh.getMesh().active_local_element_ptr_range())
  {
    // Get the subdomain name for matching
    auto sub_name = mesh.getSubdomainName(elem->subdomain_id());
    if (sub_name.empty())
      sub_name = std::to_string(elem->subdomain_id());

    auto kd_it = _kdtrees.find(sub_name);
    if (kd_it == _kdtrees.end() || !kd_it->second)
    {
      skipped_elems++;
      continue;
    }

    const auto & stored_records = _stored_data.at(sub_name);

    // Reinit to get current qp positions
    assembly.setCurrentSubdomainID(elem->subdomain_id());
    assembly.reinit(elem);
    const auto & q_points = assembly.qPoints();
    const auto n_qpoints = static_cast<unsigned int>(q_points.size());

    // For each current stateful_id that has a file match, overwrite states 1 and 2
    // only (state 0 = current will be recomputed in the first timestep).
    for (const auto file_sid : index_range(_file_props))
    {
      if (!_file_to_current_sid[file_sid])
        continue;

      const auto current_sid = *_file_to_current_sid[file_sid];
      const auto max_state = std::min(_file_props[file_sid].max_state, storage.maxState());

      // Only overwrite stateful states (1 = old, 2 = older); skip state 0 (current)
      for (unsigned int state = 1; state <= max_state; ++state)
      {
        // Assemble a blob by concatenating per-qp data from nearest stored qps.
        // This blob is compatible with PropertyValue::load() which reads n_qpoints
        // consecutive T values.
        std::stringstream assembled;

        for (unsigned int qp = 0; qp < n_qpoints; ++qp)
        {
          Point target_pt = q_points[qp];

          // Find nearest stored qp in this subdomain
          std::vector<std::size_t> return_index;
          kd_it->second->neighborSearch(target_pt, 1, return_index);

          if (return_index.empty() || return_index[0] >= stored_records.size())
            mooseError("KDTree search failed for element ",
                       elem->id(),
                       " qp ",
                       qp,
                       " in subdomain '",
                       sub_name,
                       "'.");

          const auto & nearest = stored_records[return_index[0]];

          if (file_sid < nearest.blobs.size() && state < nearest.blobs[file_sid].size())
          {
            const auto & blob = nearest.blobs[file_sid][state];
            assembled.write(blob.data(), blob.size());
          }
        }

        // Deserialize directly into the live PropertyValue in _storage
        assembled.seekg(0, std::ios::beg);
        auto & target_props = storage.setProps(elem, side, state);
        dataLoad(assembled, target_props[current_sid], nullptr);
      }
    }

    remapped_elems++;
  }

  mooseInfo("Overwrote stateful storage (states 1+) for ",
            remapped_elems,
            " elements (",
            skipped_elems,
            " skipped due to no subdomain match).");
}
