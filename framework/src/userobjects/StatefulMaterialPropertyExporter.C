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

registerMooseObject("MooseApp", StatefulMaterialPropertyExporter);

InputParameters
StatefulMaterialPropertyExporter::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription(
      "Exports stateful material property data with quadrature point positions "
      "and subdomain information to a binary file (.smatprop) for remapping onto a different "
      "mesh.");
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

void
StatefulMaterialPropertyExporter::execute()
{
  const auto & storage = _fe_problem.getMaterialPropertyStorage();

  if (!storage.hasStatefulProperties())
  {
    mooseInfo("No stateful material properties to export.");
    return;
  }

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

  // Collect per-qp data organized by subdomain name
  struct QpRecord
  {
    Point coord;
    // blobs[stateful_id][state] = binary blob for one qp value
    std::vector<std::vector<std::string>> blobs;
  };
  std::map<std::string, std::vector<QpRecord>> subdomain_data;

  const auto & props_map = storage.props(0);

  for (const auto & elem : mesh.getMesh().active_local_element_ptr_range())
  {
    if (props_map.find(elem) == props_map.end())
      continue;

    auto subdomain_name = mesh.getSubdomainName(elem->subdomain_id());
    if (subdomain_name.empty())
      subdomain_name = std::to_string(elem->subdomain_id());

    // Reinit to get physical qp positions
    assembly.setCurrentSubdomainID(elem->subdomain_id());
    assembly.reinit(elem);
    const auto & q_points = assembly.qPoints();
    const auto n_qpoints = q_points.size();

    for (unsigned int qp = 0; qp < n_qpoints; ++qp)
    {
      QpRecord rec;
      rec.coord = q_points[qp];
      rec.blobs.resize(stateful_ids.size());

      for (const auto sid : index_range(stateful_ids))
      {
        rec.blobs[sid].resize(storage.numStates());
        for (unsigned int state = 0; state < storage.numStates(); ++state)
        {
          const auto & mat_props = storage.props(elem, 0, state);
          if (sid < mat_props.size() && mat_props.hasValue(sid))
          {
            std::ostringstream blob;
            mat_props[sid].storeQp(blob, qp);
            rec.blobs[sid][state] = blob.str();
          }
        }
      }

      subdomain_data[subdomain_name].push_back(std::move(rec));
    }
  }

  // Each rank writes its own file: {base}.{rank}.smatprop
  const auto filename = _file_base + "." + std::to_string(processor_id()) + ".smatprop";
  std::ofstream out(filename, std::ios::binary);
  if (!out.good())
    mooseError("Failed to open file '", filename, "' for writing.");

  // Header
  const unsigned int magic = 0x4D504D53; // "MPMS"
  const unsigned int version = 1;
  auto n_ranks = static_cast<unsigned int>(n_processors());
  dataStore(out, magic, nullptr);
  dataStore(out, version, nullptr);
  // n_ranks is written by every rank so that the importer can discover the
  // full set of files by reading only rank 0's file.
  dataStore(out, n_ranks, nullptr);

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

  // Data organized by subdomain
  auto n_subdomains = static_cast<unsigned int>(subdomain_data.size());
  dataStore(out, n_subdomains, nullptr);

  for (auto & [sub_name, qp_records] : subdomain_data)
  {
    auto name_copy = sub_name;
    dataStore(out, name_copy, nullptr);

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

  mooseInfo("Exported stateful material properties (rank ",
            processor_id(),
            "/",
            n_processors(),
            "): ",
            n_props,
            " properties, ",
            n_subdomains,
            " subdomains to '",
            filename,
            "'.");
}
