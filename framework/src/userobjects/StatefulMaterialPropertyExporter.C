//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StatefulMaterialPropertyExporter.h"
#include "FEProblemBase.h"
#include "MooseMesh.h"
#include "MaterialPropertyStorage.h"
#include "Assembly.h"
#include "DataIO.h"

#include <fstream>
#include <sstream>

registerMooseObject("MooseApp", StatefulMaterialPropertyExporter);

InputParameters
StatefulMaterialPropertyExporter::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription(
      "Exports volumetric and boundary stateful material property data with quadrature point "
      "positions and subdomain and boundary information to a binary file (.smatprop) for "
      "remapping onto a different mesh.");
  params.addRequiredParam<std::string>("file_base",
                                       "The base name for the output file (extension "
                                       ".smatprop will be appended).");
  params.set<ExecFlagEnum>("execute_on") = EXEC_FINAL;
  return params;
}

StatefulMaterialPropertyExporter::StatefulMaterialPropertyExporter(
    const InputParameters & parameters)
  : GeneralUserObject(parameters), _file_base(getParam<std::string>("file_base"))
{
}

namespace
{
std::string
subdomainName(const MooseMesh & mesh, const SubdomainID id)
{
  const auto & name = mesh.getSubdomainName(id);
  return name.empty() ? std::to_string(id) : name;
}
}

StatefulMaterialPropertyExporter::GroupKey
StatefulMaterialPropertyExporter::volumeKey(const MooseMesh & mesh, const Elem & elem)
{
  return {subdomainName(mesh, elem.subdomain_id()), {}, ""};
}

StatefulMaterialPropertyExporter::GroupKey
StatefulMaterialPropertyExporter::faceKey(const MooseMesh & mesh,
                                          const Elem & elem,
                                          const unsigned int side)
{
  std::vector<BoundaryID> ids;
  mesh.getMesh().get_boundary_info().boundary_ids(&elem, side, ids);
  std::vector<std::string> boundary_names;
  for (const auto id : ids)
  {
    const auto & name = mesh.getBoundaryName(id);
    boundary_names.push_back(name.empty() ? std::to_string(id) : name);
  }
  std::sort(boundary_names.begin(), boundary_names.end());

  const auto neighbor = elem.neighbor_ptr(side);
  return {subdomainName(mesh, elem.subdomain_id()),
          std::move(boundary_names),
          neighbor ? subdomainName(mesh, neighbor->subdomain_id()) : ""};
}

void
StatefulMaterialPropertyExporter::execute()
{
  const auto & volume_storage = _fe_problem.getMaterialPropertyStorage();
  const auto & bnd_storage = _fe_problem.getBndMaterialPropertyStorage();

  if (!volume_storage.hasStatefulProperties() && !bnd_storage.hasStatefulProperties())
  {
    mooseInfo("No stateful material properties to export.");
    return;
  }

  // Each rank writes its own file: {base}.{rank}.smatprop
  const auto filename = _file_base + "." + std::to_string(processor_id()) + ".smatprop";
  std::ofstream out(filename, std::ios::binary);
  if (!out.good())
    mooseError("Failed to open file '", filename, "' for writing.");

  // Header
  auto magic = file_magic;
  auto version = file_version;
  auto n_ranks = static_cast<unsigned int>(n_processors());
  dataStore(out, magic, nullptr);
  dataStore(out, version, nullptr);
  // n_ranks is written by every rank so that the importer can discover the
  // full set of files by reading only rank 0's file.
  dataStore(out, n_ranks, nullptr);

  // The volumetric section precedes the boundary section
  writeStorage(out, volume_storage, /*face=*/false);
  writeStorage(out, bnd_storage, /*face=*/true);

  mooseInfo("Exported stateful material properties (rank ",
            processor_id(),
            "/",
            n_processors(),
            ") to '",
            filename,
            "'.");
}

void
StatefulMaterialPropertyExporter::writeStorage(std::ostream & out,
                                               const MaterialPropertyStorage & storage,
                                               const bool face)
{
  auto & mesh = _fe_problem.mesh();
  auto & assembly = _fe_problem.assembly(0, 0);

  const auto & stateful_ids = storage.statefulProps();

  // Gather property metadata
  struct PropMeta
  {
    std::string name;
    std::string type_str;
    unsigned int max_state;
  };
  std::vector<PropMeta> prop_meta;
  for (const auto stateful_idx : index_range(stateful_ids))
  {
    const auto prop_id = stateful_ids[stateful_idx];
    const auto & record = storage.getPropRecord(prop_id);
    auto name_opt = storage.queryStatefulPropName(prop_id);
    prop_meta.push_back({name_opt ? *name_opt : "unknown_" + std::to_string(stateful_idx),
                         record.type,
                         record.state});
  }

  // Collect per-qp data organized by group
  struct QpRecord
  {
    Point coord;
    // blobs[stateful_id][state] = binary blob for one qp value
    std::vector<std::vector<std::string>> blobs;
  };
  std::map<GroupKey, std::vector<QpRecord>> group_data;

  const auto & props_map = storage.props(0);

  for (const auto & elem : mesh.getMesh().active_local_element_ptr_range())
  {
    const auto elem_it = props_map.find(elem);
    if (elem_it == props_map.end())
      continue;

    // MaterialPropertyStorage::swapBack() can leave an element entry with no side entries for
    // elements that carry no stateful data, in which case there is nothing to loop over
    for (const auto & [side, _] : elem_it->second)
    {
      // Reinit to get physical qp positions
      assembly.setCurrentSubdomainID(elem->subdomain_id());
      if (face)
        assembly.reinit(elem, side);
      else
        assembly.reinit(elem);
      const auto & q_points = face ? assembly.qPointsFace() : assembly.qPoints();

      auto & records = group_data[face ? faceKey(mesh, *elem, side) : volumeKey(mesh, *elem)];

      for (const auto qp : make_range(q_points.size()))
      {
        QpRecord rec;
        rec.coord = q_points[qp];
        rec.blobs.resize(stateful_ids.size());

        for (const auto sid : index_range(stateful_ids))
        {
          rec.blobs[sid].resize(storage.numStates());
          for (const auto state : storage.stateIndexRange())
          {
            const auto & mat_props = storage.props(elem, side, state);
            if (sid < mat_props.size() && mat_props.hasValue(sid))
            {
              std::ostringstream blob;
              mat_props[sid].storeQp(blob, qp);
              rec.blobs[sid][state] = blob.str();
            }
          }
        }

        records.push_back(std::move(rec));
      }
    }
  }

  // Property metadata
  auto n_props = static_cast<unsigned int>(prop_meta.size());
  dataStore(out, n_props, nullptr);
  for (const auto & pm : prop_meta)
  {
    auto name = pm.name;
    auto type_str = pm.type_str;
    auto max_state = pm.max_state;
    dataStore(out, name, nullptr);
    dataStore(out, type_str, nullptr);
    dataStore(out, max_state, nullptr);
  }

  // Data organized by group
  auto n_groups = static_cast<unsigned int>(group_data.size());
  dataStore(out, n_groups, nullptr);

  for (auto & [key, qp_records] : group_data)
  {
    auto [subdomain_name, boundary_names, neighbor_subdomain_name] = key;
    dataStore(out, subdomain_name, nullptr);
    dataStore(out, boundary_names, nullptr);
    dataStore(out, neighbor_subdomain_name, nullptr);

    auto n_qpoints = static_cast<uint64_t>(qp_records.size());
    dataStore(out, n_qpoints, nullptr);

    for (const auto & rec : qp_records)
    {
      // Write coordinates
      auto pt = rec.coord;
      dataStore(out, pt, nullptr);

      // Write blobs for each property and state
      for (const auto sid : index_range(prop_meta))
      {
        for (unsigned int state = 0; state <= prop_meta[sid].max_state; ++state)
        {
          const auto & blob = (sid < rec.blobs.size() && state < rec.blobs[sid].size())
                                  ? rec.blobs[sid][state]
                                  : "";
          auto blob_size = static_cast<uint64_t>(blob.size());
          dataStore(out, blob_size, nullptr);
          if (blob_size > 0)
            out.write(blob.data(), blob_size);
        }
      }
    }
  }
}
